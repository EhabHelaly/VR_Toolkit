#include "VrCamera.h"


using namespace std;


int main()
{
    VrCamera camera;

    networkInit();
    camera.startTrackingThread(EXTERNAL_CAMERA_TRANSMITTING);
    camera.waitforTrackingThread();
    networkRelease();
    return 0;
}
