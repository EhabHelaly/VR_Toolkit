#include "VrDetection.h"
#include <fstream>
#include <sstream>
using namespace std;

//#define SHOW_CONSOLE_MESSAGES

VrDetection::VrDetection()
{
}
bool VrDetection::init()
{
    return  _loadPM() &&
            _cam1.startTrackingThread(INTERNAL_CAMERA_ID) &&
            _cam2.startTrackingThread(EXTERNAL_CAMERA_RECEIVING) ;
}
void VrDetection::release()
{
    _cam1.stopTrackingThread();
    _cam2.stopTrackingThread();
}
bool VrDetection::_camerasAreConnected()
{
    return _cam1.isThreadRunning() && _cam2.isThreadRunning();
}
bool VrDetection::_loadPM()
{
    ifstream inFile;
    inFile.open("Data/calibration.calib");
    if (!inFile.is_open())
        return false;
    string line;
    getline(inFile,line);
    stringstream stream(line);
    stream>> _projMat1.at<double>(0,0);
    stream>> _projMat1.at<double>(0,1);
    stream>> _projMat1.at<double>(0,2);
    stream>> _projMat1.at<double>(0,3);
    stream>> _projMat1.at<double>(1,0);
    stream>> _projMat1.at<double>(1,1);
    stream>> _projMat1.at<double>(1,2);
    stream>> _projMat1.at<double>(1,3);
    stream>> _projMat1.at<double>(2,0);
    stream>> _projMat1.at<double>(2,1);
    stream>> _projMat1.at<double>(2,2);
    stream>> _projMat1.at<double>(2,3);

    stream>> _projMat2.at<double>(0,0);
    stream>> _projMat2.at<double>(0,1);
    stream>> _projMat2.at<double>(0,2);
    stream>> _projMat2.at<double>(0,3);
    stream>> _projMat2.at<double>(1,0);
    stream>> _projMat2.at<double>(1,1);
    stream>> _projMat2.at<double>(1,2);
    stream>> _projMat2.at<double>(1,3);
    stream>> _projMat2.at<double>(2,0);
    stream>> _projMat2.at<double>(2,1);
    stream>> _projMat2.at<double>(2,2);
    stream>> _projMat2.at<double>(2,3);

    inFile.close();
#ifdef SHOW_CONSOLE_MESSAGES
    cout<<"Loaded Calibration matrices"<<endl;
    std::cout << "Projection Matrix 1 =  " << std::endl;
    std::cout << _projMat1 << std::endl << std::endl;
    //Get Projection Matrix
    std::cout << "Projection Matrix 2 =  " << std::endl;
    std::cout << _projMat2 << std::endl << std::endl;
#endif // SHOW_CONSOLE_MESSAGES

    return true;
}
bool VrDetection::getPosition()
{
        if (!_camerasAreConnected())
            return false;

        headMat1.at<double>(0,0) = _cam1.getX1();
        headMat1.at<double>(1,0) = _cam1.getY1();
        headMat2.at<double>(0,0) = _cam2.getX1();
        headMat2.at<double>(1,0) = _cam2.getY1();

        handMat1.at<double>(0,0) = _cam1.getX2();
        handMat1.at<double>(1,0) = _cam1.getY2();
        handMat2.at<double>(0,0) = _cam2.getX2();
        handMat2.at<double>(1,0) = _cam2.getY2();

        if(headMat1.at<double>(0,0) != -1 && headMat2.at<double>(0,0)!= -1 )
        {
            //if both cameras can see the head blob then triangulate the points
            cv::triangulatePoints(_projMat1, _projMat2, headMat1, headMat2, headpnt3D);

            if(headpnt3D.at<double>(3,0) != 0){
                pos1New.x = headpnt3D.at<double>(0,0) / headpnt3D.at<double>(3,0);
                pos1New.y = headpnt3D.at<double>(1,0) / headpnt3D.at<double>(3,0);
                pos1New.z = headpnt3D.at<double>(2,0) / headpnt3D.at<double>(3,0);
            }
        }

        if(handMat1.at<double>(0,0) != -1 && handMat2.at<double>(0,0)!= -1 )
        {
            //if both cameras can see the hand blob then triangulate the points
            cv::triangulatePoints(_projMat1, _projMat2, handMat1, handMat2, handpnt3D);

            if(handpnt3D.at<double>(3,0) != 0){
                pos2New.x = handpnt3D.at<double>(0,0) / handpnt3D.at<double>(3,0);
                pos2New.y = handpnt3D.at<double>(1,0) / handpnt3D.at<double>(3,0);
                pos2New.z = handpnt3D.at<double>(2,0) / handpnt3D.at<double>(3,0);
            }

            handIsFound = true;
        }
        else
        {
            handIsFound = false;
        }




#ifdef SHOW_CONSOLE_MESSAGES
        std::cout << "Reconstructed 3D point of head in homogeneous coordinates: " << headpnt3D << std::endl;
        std::cout << "Reconstructed 3D point of hand in homogeneous coordinates: " << handpnt3D << std::endl;

        std::cout << "head_cam1X= " << headMat1.at<double>(0,0)
                << " ,head_cam1Y= " << headMat1.at<double>(1,0)
                << " ,head_Cam2X= " << headMat2.at<double>(0,0)
                << " ,head_Cam2Y= " << headMat2.at<double>(1,0)
                << std::endl;
        //std::cout << "headX= " << pos1New.x << " ,headY= " << pos1New.y << " ,headZ= " << pos1New.z << std::endl << std::endl;

        std::cout << "hand_cam1X= " << handMat1.at<double>(0,0)
                << " ,hand_cam1Y= " << handMat1.at<double>(1,0)
                << " ,hand_Cam2X= " << handMat2.at<double>(0,0)
                << " ,hand_Cam2Y= " << handMat2.at<double>(1,0)
                << std::endl;
        std::cout << "handX= " << pos2New.x << " ,handY= " << pos2New.y << " ,handZ= " << pos2New.z << std::endl << std::endl;*/
#endif // SHOW_CCONSOLE_MESSAGES

        //for the first readings -> set x, y, z to be zero
        if (_headfirst == 0) { pos1Old=pos1New; _headfirst++; }
        if (_handfirst == 0) { pos2Old=pos2New; _handfirst++; }

        pos1Delta=pos1New-pos1Old;
        pos1Old=pos1New;
        pos1Delta.z = -pos1Delta.z ;

        pos2Delta=pos2New-pos2Old;
        pos2Old=pos2New;
        pos2Delta.z = -pos2Delta.z ;
        return true;
}
