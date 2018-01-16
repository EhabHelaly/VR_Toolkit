#include <iostream>
#include "VrCalibration.h"

using namespace std;

int main()
{
    cout<<"--------------------------------------------------------------------------------"<<endl;
    cout<<"---------------------------  VR Cameras Calibration  ---------------------------"<<endl;
    cout<<"--------------------------------------------------------------------------------"<<endl;
    cout<<"- Must be executed after setting up toolkit for proper position tracking"<<endl;
    cout<<"- Performs Extrinsic parameters calibration for cameras"<<endl;
    cout<<"- Intrinsic parameters were calculated using Chessboard Calibration method and are hard-coded in algorithm"<<endl;
    cout<<"- Generates Data/calibration.calib file which holds cameras' projection matrices"<<endl;
    cout<<"- Data/points.calib file holds calibration points, can be edited before calibration"<<endl;

    std::cout <<std::endl<< "Start of calibration" << std::endl;

    VrCalibration cameras;

    cameras.init();
    cameras.calibrate();
    cameras.release();

    std::cout << "End of calibration" << std::endl;
    return 0;
}
