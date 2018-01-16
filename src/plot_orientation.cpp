#include <iostream>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include "plot.hpp"

#include "network.h"
#include "VrGadget.h"

using namespace std;
using namespace cv;

void makePlot(Mat &display,Mat xData, Mat &yData, double nextData, Scalar color=Scalar(0,255,255))
{
    Ptr<plot::Plot2d> plot;

    memcpy(yData.data,yData.data+8,(yData.cols-1)*8);
    yData.at<double>(yData.cols-1) = nextData;

    plot = plot::createPlot2d(xData, yData);
    plot->setPlotSize(650, 650);
    plot->setMaxX(yData.cols);
    plot->setMinX(-yData.cols/30);
    plot->setMaxY(-360);
    plot->setMinY(360);
    plot->setPlotLineColor(color);
    plot->render(display);

}
int main()
{

    VrGadget sensor;
    networkInit();
    sensor.init(5556);
    Mat display1;
    Mat display2;
    Mat display3;
    Mat display;
    Mat yData(1, 1000, CV_64F,Scalar(0));
    Mat xData(1, yData.cols, CV_64F);
    for (int elem=0;elem<yData.cols;elem++)
        xData.at<double>(elem) = elem;

    Mat y1Data(1, 1000, CV_64F,Scalar(0));
    Mat y2Data(1, 1000, CV_64F,Scalar(0));
    Mat y3Data(1, 1000, CV_64F,Scalar(0));

    while(waitKey(13)!=27)
    {
        sensor.getRotation();
        makePlot(display1,xData,y1Data, sensor.R.x);
        makePlot(display2,xData,y2Data, sensor.R.y,Scalar(255,0,0));
        makePlot(display3,xData,y3Data, sensor.R.z,Scalar(0,255,0));
        add(display1,display2,display);
        add(display ,display3,display);
        imshow("Plot", display);
    }

    sensor.release();
    networkRelease();

    return 0;
}
