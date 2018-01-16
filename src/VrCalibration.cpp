#include "VrCalibration.h"
#include "VrCamera.h"
#include <fstream>
#include <sstream>

using namespace cv;
using namespace std;

/// class Cam
Cam::Cam()
{
}
bool Cam::loadPoints(){
    ifstream infile;
    infile.open("Data/points.calib");

    if (!infile.is_open())
    {
        return false;
    }
    string line;
    while (getline(infile,line))
    {
        stringstream stream(line);
        Point3d p;
        stream >> p.x >> p.y >> p.z;
        _pts3d.push_back(p);
    }
    infile.close();
    cout<<"Loaded "<<_pts3d.size()<<" points"<<endl;
    return true;
}
void Cam::calculateRT(){
    cv::solvePnP(_pts3d, _pts2d, _camMatrix, _disCoeff, _rvec, _tvec, false,  cv::SOLVEPNP_ITERATIVE);
}
void Cam::projectPoint(){

    /*float u, v, w;
    std::cout << "Enter 3D point to be projected:" << std::endl;
    std::cin >> u; std::cin >> v; std::cin >> w;

    std::vector<cv::Point3d> point3D;
    std::vector<cv::Point2d> point2D;

    cv::Point3d point(u, v, w);
    point3D.push_back(point);

    projectPoints(point3D, _rvec, _tvec, _camMatrix, _disCoeff, point2D);
    std::cout << point2D << std::endl;*/

    /******************** Another method ****************************/

    //Build the projection matrix
    //then multiply it by the 3D point
    //As the result is a homogeneous matrix : so x=x/z & y=y/z

    cv::Mat rotM(3, 3, CV_64F);
    Rodrigues(_rvec, rotM);

    // push tvec to transposed Mat
    cv::Mat rotMT = rotM.t();
    rotMT.push_back(_tvec.reshape(1, 1));

    // transpose back, and multiply
    cv::Mat projMat = _camMatrix * rotMT.t();
    _projMat = projMat;

    /*cv::Mat point_3D = (cv::Mat_<double>(4,1) << u,v,w,1);

    cv::Mat point_2D(3, 1, CV_64FC1);
    point_2D = projMat * point_3D;

    double x = point_2D.at<double>(0,0) / point_2D.at<double>(2,0);
    double y = point_2D.at<double>(1,0) / point_2D.at<double>(2,0);

    std::cout << "[" << x << ", " << y << "]" << std::endl;*/
}

VrCalibration::VrCalibration(){

}
void VrCalibration::init(){
    networkInit();
    _cam1.track.startTrackingThread(INTERNAL_CAMERA_ID);
    _cam2.track.startTrackingThread(EXTERNAL_CAMERA_RECEIVING);
}
void VrCalibration::calibrate(){
    _cam1.loadPoints();
    _cam2.loadPoints();

    if (_cam1._pts3d.size()==0)
    {
        cout<<"No 3D points found"<<endl;
        return;
    }

    unsigned int point=0;
    while(point<_cam1._pts3d.size())
    {
        std::cout << "\nNext 3D point: "<< _cam1._pts3d[point].x << ", "
                                        << _cam1._pts3d[point].y << ", "
                                        << _cam1._pts3d[point].z << std::endl;

        system("pause");
        _cam1._pts2d.push_back(cv::Point2d(_cam1.track.getX1(), _cam1.track.getY1()));
        _cam2._pts2d.push_back(cv::Point2d(_cam2.track.getX1(), _cam2.track.getY1()));
        std::cout <<"CAM 1 : "<< _cam1.track.getX1() << " ," << _cam1.track.getY1() << std::endl;
        std::cout <<"CAM 2 : "<< _cam2.track.getX1() << " ," << _cam2.track.getY1() << std::endl;

        point++;
    }

    _cam1.calculateRT();
    _cam2.calculateRT();
    _cam1.projectPoint();
    _cam2.projectPoint();

    std::cout << "Projection Matrix 1 =  " << std::endl;
    std::cout << _cam1.getProjMat() << std::endl << std::endl;
    //Get Projection Matrix
    std::cout << "Projection Matrix 2 =  " << std::endl;
    std::cout << _cam2.getProjMat() << std::endl << std::endl;

    _writePM();
}
void VrCalibration::release(){
    _cam1.track.stopTrackingThread();
    _cam2.track.stopTrackingThread();
    networkRelease();
}
bool VrCalibration::_writePM(){
    ofstream outFile;
    outFile.open("Data/calibration.calib");
    if (!outFile.is_open())
    {
        return false;
    }

    outFile<< _cam1.getProjMat().at<double>(0,0); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(0,1); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(0,2); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(0,3); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(1,0); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(1,1); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(1,2); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(1,3); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(2,0); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(2,1); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(2,2); outFile<<" ";
    outFile<< _cam1.getProjMat().at<double>(2,3); outFile<<" ";

    outFile<< _cam2.getProjMat().at<double>(0,0); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(0,1); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(0,2); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(0,3); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(1,0); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(1,1); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(1,2); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(1,3); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(2,0); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(2,1); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(2,2); outFile<<" ";
    outFile<< _cam2.getProjMat().at<double>(2,3); outFile<<" ";

    outFile.close();
    cout<<"Saved calibrated data to ""calibration.calib"" "<<endl;

    return true;
}





