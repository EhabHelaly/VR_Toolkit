#include "network.h"
#include "VrCamera.h"
#include <dshow.h>
#include <string>

#define SOCKET_PORT 1234
#define THREAD_DELAY_TIME 30

using namespace cv;
using namespace std;

static DWORD trackBlob(LPVOID* track);
static int getCamID(string cameraName);
static int getDeviceInformation(IEnumMoniker *pEnum, string cameraName);
static HRESULT enumerateDevices(REFGUID category, IEnumMoniker **ppEnum);


VrCamera::VrCamera()
{

}
bool VrCamera::startTrackingThread(int ID)
{
    id=ID;
    _threadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) trackBlob,this,0,&_threadID);
    return _threadHandle!=NULL;
}
bool VrCamera::waitforTrackingThread()
{
    WaitForMultipleObjects(1, &_threadHandle, TRUE, INFINITE);

    // Close all thread handles
    CloseHandle(_threadHandle);
    return true;
}
bool VrCamera::stopTrackingThread()
{
    _stopThread=true;
    return waitforTrackingThread();
}
bool VrCamera::isThreadRunning()
{
    return _threadIsRunning;
}
void VrCamera::setParameters()
{
    initKalman();

    // Setup SimpleBlobDetector parameters.
    cv::SimpleBlobDetector::Params params;

    // Change thresholds
    params.minThreshold = 100;
    params.maxThreshold = 255;
    params.filterByColor = false;
    //params.blobColor = 255;
    // Filter by Area.
    params.filterByArea = true;
    params.minArea = 5;
    // Filter by Circularity
    //params.filterByCircularity = true;
    //params.minCircularity = 0.1;
    // Filter by Convexity
    params.filterByConvexity = false;
    //params.minConvexity = 0.87;
    // Filter by Inertia
    params.filterByInertia = false;
    //params.minInertiaRatio = 0.01;
    // Set up detector with params
    _detector = cv::SimpleBlobDetector::create(params);
}
void VrCamera::initKalman(){
    _kf.init(_stateSize, _measSize, _contrSize, CV_64F);

    cv::setIdentity(_kf.transitionMatrix);
    _kf.measurementMatrix = cv::Mat::zeros(_measSize, _stateSize, CV_64F);
    _kf.measurementMatrix.at<double>(0) = 1.0f;
    _kf.measurementMatrix.at<double>(5) = 1.0f;

    //setIdentity(_kf.processNoiseCov, cv::Scalar::all(1e-2));

    _kf.processNoiseCov.at<double>(0) = 1e-2;
    _kf.processNoiseCov.at<double>(5) = 1e-2;
    _kf.processNoiseCov.at<double>(10) = 2.0f;
    _kf.processNoiseCov.at<double>(15) = 1.0f;

    setIdentity(_kf.measurementNoiseCov, cv::Scalar::all(1e-1));
    //setIdentity(_kf.errorCovPost, cv::Scalar::all(.1));

}
float VrCamera::getDistSq(cv::Point point1, cv::Point point2){
    // SqrtDistance = (x1-x2)^2 + (y1-y2)^2
    return std::pow((point1.x- point2.x),2) + std::pow((point1.y - point2.y),2);
}
unsigned int VrCamera::getID(){
    unsigned int i=0;
    for(i=0; i < ids.size(); i++){
        std::unordered_map<unsigned int, bool>::iterator it = ids.find(i);
        //if not found or it's boolean = false
        if(it == ids.end() || it->second == false){
            it->second = true;
            return i;
        }
        //found or it's boolean = true
        continue;
    }
    ids[i] = true;
    return i;
}
void VrCamera::deleteID(unsigned int id){
    std::unordered_map<unsigned int, bool>::iterator it = ids.find(id);
    it->second = false;
}
void VrCamera::detectBlobs(){

    double precTick = _ticks;
    _ticks = (double) cv::getTickCount();
    double dT = (_ticks - precTick) / cv::getTickFrequency(); //seconds

    if(_found) {

            // >>>> Matrix A
            _kf.transitionMatrix.at<double>(2) = dT;
            _kf.transitionMatrix.at<double>(7) = dT;
            // <<<< Matrix A

            _state = _kf.predict();

            _centerKalman.x = _state.at<double>(0);
            _centerKalman.y = _state.at<double>(1);
    }

    // Detect blobs
    /*Mat bin;
    inRange(_frame,Scalar(50,50,50),Scalar(255,255,255),bin);
    Mat array[3]={bin,bin,bin};
    merge(array,3,_frame);*/
    _detector->detect(_frame, _currentBlobs);
    drawKeypoints(_frame, _currentBlobs, _frameDetected, cv::Scalar(0,0,255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

    //Match the blobs
    if(_oldBlobs.size() == 0 && _currentBlobs.size() > 0){
        _oldBlobs = _currentBlobs;
        //make sure head id = 0
        std::sort(_oldBlobs.begin(), _oldBlobs.end(), [](const cv::KeyPoint& lhs, const cv::KeyPoint& rhs){ return lhs.pt.y < rhs.pt.y; });

        for (unsigned int i=0; i < _oldBlobs.size(); i++){
            _oldBlobs[i].class_id = getID();
        }
    }
    //the same number of blobs OR new blobs in the current frame
    else if(_oldBlobs.size() <= _currentBlobs.size()){
            //Match whatever blobs you can match
            for(unsigned int i=0; i < _oldBlobs.size(); i++){
                unsigned int temp;
                cv::KeyPoint match;
                float distance = 10000000000;
                for(unsigned int j=0; j < _currentBlobs.size(); j++){
                    float d = getDistSq(_oldBlobs[i].pt, _currentBlobs[j].pt);
                    if(d < distance && !_currentBlobs[j].response){
                        distance = d;
                        temp = _oldBlobs[i].class_id;
                        match = _currentBlobs[j];
                        match.class_id = temp;
                        temp = j;
                    }
                }
                _currentBlobs[temp].response = 1;
                _oldBlobs[i] = match;
            }

            // Whatever is leftover make new blobs
            for(unsigned int i=0; i < _currentBlobs.size(); i++){
                if(!_currentBlobs[i].response){
                    _currentBlobs[i].class_id = getID();
                    _oldBlobs.push_back(_currentBlobs[i]);
                }
            }
        }

    //some blobs disappeared in the current frame
    else if(_oldBlobs.size() > _currentBlobs.size()){

        for(unsigned int i=0; i < _oldBlobs.size(); i++){
             _oldBlobs[i].response = 0;
        }

        //Match whatever blobs you can match
        for(unsigned int i=0; i < _currentBlobs.size(); i++){
            unsigned int temp;
            cv::KeyPoint match;
            float distance = 10000000000;
            for(unsigned int j=0; j < _oldBlobs.size(); j++){
                float d = getDistSq(_oldBlobs[j].pt, _currentBlobs[i].pt);
                 if(d < distance && !_oldBlobs[j].response){
                    distance = d;
                    temp = _oldBlobs[j].class_id;
                    match = _currentBlobs[i];
                    match.class_id = temp;
                    _oldBlobs[j] = match;
                    temp = j;
                }
            }
            _oldBlobs[temp].response = 1;
        }

        for(unsigned int i=0; i < _oldBlobs.size(); i++){
           if(!_oldBlobs[i].response){
            deleteID(_oldBlobs[i].class_id);
            _oldBlobs[i] = _oldBlobs.back();
            _oldBlobs.pop_back();
            i--;
           }
        }
    }


    if(_oldBlobs.size() !=0){
            for (unsigned int i=0; i < _oldBlobs.size(); i++){
                cv::putText(_frameDetected,
                "ID="+std::to_string(_oldBlobs[i].class_id),
                cv::Point(_oldBlobs[i].pt.x ,_oldBlobs[i].pt.y),
                3,
                0.8,
                cv::Scalar(0,255,255));

                if(_oldBlobs[i].class_id == 0) {
                    _head = _oldBlobs[i].pt;
                    _headIsFound = true;
                }else if (_oldBlobs[i].class_id == 1){
                    _hand = _oldBlobs[i].pt;
                    _handIsFound = true;
                }
            }
    }else {
        _head = {-1, -1};
        _hand = {-1, -1};
    }

    if(!_headIsFound){
        _head = {-1,-1};
        _notFoundCount++;
        if( _notFoundCount >= 10 )
        {
            _found = false;
        }
        else
            _kf.statePost = _state;
    }else{

        _notFoundCount = 0;
        _meas.at<double>(0) = _head.x;
        _meas.at<double>(1) = _head.y;

        if (!_found) // First detection!
        {
            // Initialization
            _kf.errorCovPre.at<double>(0)  = 1;
            _kf.errorCovPre.at<double>(5)  = 1;
            _kf.errorCovPre.at<double>(10) = 1;
            _kf.errorCovPre.at<double>(15) = 1;

            _state.at<double>(0) = _meas.at<double>(0);
            _state.at<double>(1) = _meas.at<double>(1);
            _state.at<double>(2) = 0;
            _state.at<double>(3) = 0;

            _kf.statePost = _state;

            _found = true;
        }
        else
            _kf.correct(_meas); // Kalman Correction

        }

        if(!_handIsFound) _hand = {-1,-1};
        _headIsFound=false; _handIsFound=false;

}

DWORD VrCamera::trackingThread()
{
    VideoCapture cam(getCamID("PS3Eye Camera"));

    if (!cam.isOpened())
    {
        cout<<"Failed to open Camera"<<endl;
        return -1;
    }
    cam.set(CAP_PROP_FRAME_WIDTH,640);
    cam.set(CAP_PROP_FRAME_HEIGHT,480);
    cam.set(CAP_PROP_FPS,60);


    setParameters();

    _threadIsRunning=true;
    while (!_stopThread && waitKey(1)!=27)
    {
        cam>>_frame;
        if (_frame.empty())
        {
            std::cout << "Failed to obtain camera frame!" << std::endl;
        }
        else
        {
            detectBlobs();
            imshow("iVR",_frameDetected);
        }
        Sleep(THREAD_DELAY_TIME);
        //SwitchToThread();
    }
    _threadIsRunning=false;
    return 0;
}
DWORD VrCamera::transmittingThread()
{
    // detecetion
    VideoCapture cam(getCamID("PS3Eye Camera"));
    // network
    UDPsocket socket;
    union Data{
        char buffer[DEFAULT_BUFLEN];
        double number[4];
    };
    Data data;

    if (!cam.isOpened())
    {
        cout<<"Failed to open Camera"<<endl;
        return -1;
    }
    cam.set(CAP_PROP_FRAME_WIDTH,640);
    cam.set(CAP_PROP_FRAME_HEIGHT,480);
    cam.set(CAP_PROP_FPS,60);
    if (!socket.initSender(SOCKET_PORT))
    {
        cout<<"Failed Connecting Cameras"<<endl;
        socket.release();
        return -1;
    }


    setParameters();
    _threadIsRunning=true;
    while (!_stopThread && waitKey(1)!=27)
    {
        cam>>_frame;
        if (_frame.empty())
        {
            std::cout << "Failed to obtain camera frame!" << std::endl;
        }
        else
        {
            detectBlobs();
            imshow("iVR",_frameDetected);
            data.number[0]=_head.x;
            data.number[1]=_head.y;
            data.number[2]=_hand.x;
            data.number[3]=_hand.y;
            socket.transmit(data.buffer,4*sizeof(double));
        }
        Sleep(THREAD_DELAY_TIME);
        //SwitchToThread();
    }
    socket.release();
    _threadIsRunning=false;
    return 0;
}
DWORD VrCamera::receivingThread()
{
    UDPsocket socket;
    union Data{
        char buffer[DEFAULT_BUFLEN];
        double number[4];
    };
    Data data;
    int size;

    if (!socket.initReceiver(SOCKET_PORT))
    {
        cout<<"Failed Connecting Cameras"<<endl;
        socket.release();
        return -1;
    }

    _threadIsRunning=true;
    while (!_stopThread)
    {
        if (socket.receive(data.buffer,size))
        {
            if (size==4*sizeof(double))
            {
                _head.x=data.number[0];
                _head.y=data.number[1];
                _hand.x=data.number[2];
                _hand.y=data.number[3];
            }
        }
        Sleep(THREAD_DELAY_TIME);
        //SwitchToThread();
    }
    socket.release();
    _threadIsRunning=false;
    return 0;
}

//////////////////////////////////////////////////////////////////
static DWORD trackBlob(LPVOID* track)
{
    switch (((VrCamera*)track)->id)
    {
        case EXTERNAL_CAMERA_TRANSMITTING:
            return ((VrCamera*)track)->transmittingThread();
        case EXTERNAL_CAMERA_RECEIVING:
            return ((VrCamera*)track)->receivingThread();
        default:
            return ((VrCamera*)track)->trackingThread();
    }
}

static HRESULT enumerateDevices(REFGUID category, IEnumMoniker **ppEnum)
{
    // Create the System Device Enumerator.
    ICreateDevEnum *pDevEnum;
    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pDevEnum));

    if (SUCCEEDED(hr))
    {
        // Create an enumerator for the category.
        hr = pDevEnum->CreateClassEnumerator(category, ppEnum, 0);
        if (hr == S_FALSE)
        {
            hr = 0;//VFW_E_NOT_FOUND;  // The category is empty. Treat as an error.
        }
        pDevEnum->Release();
    }
    return hr;
}
static int getDeviceInformation(IEnumMoniker *pEnum, string cameraName)
{
    IMoniker *pMoniker = NULL;
    int cameraID=-1;
    int ID=0;

    while (pEnum->Next(1, &pMoniker, NULL) == S_OK && cameraID<0)
    {
        IPropertyBag *pPropBag;
        HRESULT hr = pMoniker->BindToStorage(0, 0, IID_PPV_ARGS(&pPropBag));
        if (FAILED(hr))
        {
            pMoniker->Release();
            continue;
        }

        VARIANT var;
        VariantInit(&var);

        // Get description or friendly name.
        hr = pPropBag->Read(L"Description", &var, 0);
        if (FAILED(hr))
        {
            hr = pPropBag->Read(L"FriendlyName", &var, 0);
        }
        if (SUCCEEDED(hr))
        {
            wstring wname(var.bstrVal);
            string devName(wname.begin(),wname.end());
            if (!devName.compare(cameraName))
            {
                //printf("%s  ID: %d\n", devName.c_str(),cameraID);
                cameraID=ID;
            }
            ID++;
            VariantClear(&var);
        }

        pPropBag->Release();
        pMoniker->Release();
    }
    return cameraID;
}

static int getCamID(string cameraName)
{
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    int cameraID=-1;
    if (SUCCEEDED(hr))
    {
        IEnumMoniker *pEnum;

        hr = enumerateDevices(CLSID_VideoInputDeviceCategory, &pEnum);
        if (SUCCEEDED(hr))
        {
            cameraID=getDeviceInformation(pEnum,cameraName);
            pEnum->Release();
        }
        CoUninitialize();
    }
    return cameraID;
}
