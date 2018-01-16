#ifndef VR_CAMERA_H
#define VR_CAMERA_H

#include "network.h"

#include <windows.h>
#include <opencv2/opencv.hpp>
#include <unordered_map>

#define INTERNAL_CAMERA_ID            1
#define EXTERNAL_CAMERA_RECEIVING    -2
#define EXTERNAL_CAMERA_TRANSMITTING -3

class VrCamera
{
    public:
        VrCamera();

        bool startTrackingThread(int ID);
        bool waitforTrackingThread();
        bool stopTrackingThread();

        // thread functions
        DWORD     trackingThread();
        DWORD    receivingThread();
        DWORD transmittingThread();

        //getters
        double getX1() { return _head.x; }
        double getY1() { return _head.y; }
        double getX2() { return _hand.x; }
        double getY2() { return _hand.y; }
        bool isThreadRunning();

        int id;

    private:
        cv::Mat _frame;
        cv::Mat _frameDetected;

        cv::Point _head = {0,0};
        cv::Point _hand = {0,0};

        bool      _stopThread=false;
        bool _threadIsRunning=false;
        bool _handIsFound    =false;
        bool _headIsFound    =false;

        HANDLE  _threadHandle;
        DWORD   _threadID;

        void setParameters();
        void initKalman();
        void detectBlobs();
        unsigned int getID();
        void deleteID(unsigned int id);
        float getDistSq(cv::Point point1, cv::Point point2);

        // Storage for blobs
        std::vector<cv::KeyPoint> _currentBlobs;
        std::vector<cv::KeyPoint> _oldBlobs;
        cv::Ptr<cv::SimpleBlobDetector> _detector;

        std::unordered_map<unsigned int, bool> ids;

        //kalman filter
        int _stateSize = 4;
        int _measSize  = 2;
        int _contrSize = 0;
        cv::KalmanFilter _kf;

        cv::Mat _state = cv::Mat(_stateSize, 1, CV_64F);  // [x,y,v_x,v_y]
        cv::Mat _meas  = cv::Mat(_measSize , 1, CV_64F);  // [x,y]

        bool _found = false;
        int _notFoundCount = 0;
        double _ticks = 0;
        cv::Point _centerKalman = {0,0};


};

#endif // VR_CAMERA_H
