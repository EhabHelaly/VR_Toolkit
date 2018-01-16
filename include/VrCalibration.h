#ifndef VR_CALIBRATION_H
#define VR_CALIBRATION_H

#include "network.h"
#include "VrCamera.h"

#include <windows.h>
#include <opencv2/opencv.hpp>

class Cam{
public:
    Cam();
    bool loadPoints();
    void calculateRT();
    void projectPoint();
    cv::Mat getProjMat() { return _projMat; };
    VrCamera track;

    //2d -> 3d correspondence
    std::vector<cv::Point2d> _pts2d;
    std::vector<cv::Point3d> _pts3d;

private:
    //Intrinsic Parameters
    cv::Mat _camMatrix = (cv::Mat_<double>(3,3) <<
                            5.6070e+002,   0.           ,     3.2952e+002,
                            0.          ,   5.7610e+002 ,     2.4862e+002,
                            0.          ,   0.           ,     1.0
                            );

    //Distortion Parameters
    cv::Mat _disCoeff = (cv::Mat_<double>(1,4) <<
                            0, 0, 0., 0.
                            );

    cv::Mat _projMat;
    // Output rotation and translation matrix
    cv::Mat _rvec, _tvec;
};

class VrCalibration
{
    public:
        VrCalibration();
        void init();
        void calibrate();
        void release();

    private:
        Cam _cam1;
        Cam _cam2;
        bool _writePM();

};

#endif // VR_CALIBRATION_H
