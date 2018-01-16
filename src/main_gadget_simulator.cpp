#include <iostream>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include "network.h"

using namespace std;
using namespace cv;

int main()
{
    networkInit();

    UDPsocket esp;
    if (!esp.initSender(80))
        return -1;

    union Data{
        char buffer[14]; // 14
        struct Arg{
            float rot[3]; // 12
            char    b;// 1
            char stop;// 1
        }arg;
    }data;
    data.arg.rot[0]=0;
    data.arg.rot[1]=0;
    data.arg.rot[2]=0;
    data.arg.b=0;
    data.arg.stop='\n';


    namedWindow("keyboard events");
    float speed=0.1;
    bool hold=false;


    char key;
    while((key=waitKey(15))!=27)
    {
        // clear button states , comment if needed
        if (!hold)
            data.arg.b=0;
        switch (key)
        {
            case '1': {data.arg.b^=1; break;}
            case '2': {data.arg.b^=2; break;}
            case '3': {hold^=1;break;}

            case 'q': {data.arg.rot[0]-=speed;if (data.arg.rot[0]<-3.14) {data.arg.rot[0]+=(2*3.14);} break;}
            case 'w': {data.arg.rot[0]+=speed;if (data.arg.rot[0]> 3.14) {data.arg.rot[0]-=(2*3.14);} break;}

            case 'a': {data.arg.rot[1]-=speed;if (data.arg.rot[1]<-3.14) {data.arg.rot[1]+=(2*3.14);} break;}
            case 's': {data.arg.rot[1]+=speed;if (data.arg.rot[1]> 3.14) {data.arg.rot[1]-=(2*3.14);} break;}

            case 'z': {data.arg.rot[2]-=speed;if (data.arg.rot[2]<-3.14) {data.arg.rot[2]+=(2*3.14);} break;}
            case 'x': {data.arg.rot[2]+=speed;if (data.arg.rot[2]> 3.14) {data.arg.rot[2]-=(2*3.14);} break;}

        }
        esp.transmit(data.buffer,14);

        Mat image(200,800,CV_8UC3,Scalar(255,255,255));
        char* text=new char[100];
        sprintf(text,"X:%.3f Y:%.3f Z:%.3f B0:%d B1:%d Hold: %s",data.arg.rot[0],data.arg.rot[1],data.arg.rot[2],data.arg.b&1,data.arg.b&2,hold? "on":"off");
        putText(image,text,Point(10,100),3,1.0,Scalar(0,250,0),1,LINE_AA);
        imshow("keyboard events",image);
        delete text;
    }
    esp.release();
    networkRelease();

    return 0;
}
