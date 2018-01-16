#ifndef VR_DETECTION_H
#define VR_DETECTION_H

#include "VrCamera.h"
#include <glm/glm.hpp>

class VrDetection
{
    public:
        VrDetection();
        bool init();
        bool getPosition();
        void release();
        cv::Mat headpnt3D = cv::Mat(4, 1, CV_64F); //reconstructed 3D point in homogeneous coordinates
        cv::Mat handpnt3D = cv::Mat(4, 1, CV_64F); //reconstructed 3D point in homogeneous coordinates

        cv::Mat headMat1=cv::Mat(2, 1, CV_64F);
        cv::Mat headMat2=cv::Mat(2, 1, CV_64F);
        cv::Mat handMat1=cv::Mat(2, 1, CV_64F);
        cv::Mat handMat2=cv::Mat(2, 1, CV_64F);

        // head
        glm::vec3 pos1Old  =glm::vec3(0);           // old readings
        glm::vec3 pos1New  =glm::vec3(0);           // current readings
        glm::vec3 pos1Delta=glm::vec3(0);           // delta
        // hand
        glm::vec3 pos2Old  =glm::vec3(0);
        glm::vec3 pos2New  =glm::vec3(0);
        glm::vec3 pos2Delta=glm::vec3(0);
        bool handIsFound = false;                    //true if both cameras see the hand blob


    private:
        int _headfirst = 0;
        int _handfirst = 0;
        VrCamera _cam1;
        VrCamera _cam2;
        cv::Mat _projMat1=cv::Mat(3, 4, CV_64F);
        cv::Mat _projMat2=cv::Mat(3, 4, CV_64F);
        bool _loadPM();
        bool _camerasAreConnected();

};

#endif // VR_DETECTION_H
