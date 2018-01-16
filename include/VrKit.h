#ifndef VR_KIT_H
#define VR_KIT_H

#include "network.h"
#include "VrGadget.h"
#include "VrDetection.h"

#include <iostream>
#include <string>
#include <vector>
#include <Windows.h>


#define VR_POSITION_TRACKING    1
#define VR_ORIENTATION_TRACKING 2

using namespace std;

class VrKit
{
    public:
        VrKit();
        bool init(int mode=(VR_POSITION_TRACKING|VR_ORIENTATION_TRACKING) );
        void getNewData();
        void release();

        // getters
        glm::vec3 getHeadPos();
        glm::vec3 getHeadPosD();

        glm::vec3 getHandPos();
        glm::vec3 getHandPosD();

        glm::vec3 getHeadRot();
        glm::vec3 getHeadRotD();

        glm::vec3 getHandRot();
        glm::vec3 getHandRotD();

        bool buttonIsDown(int b);
        bool buttonIsUp  (int b);
        bool buttonIsOn  (int b);
        bool buttonIsOff (int b);


    private:
        VrGadget _handGadget;
        VrGadget _headGadget;
        VrDetection _headset;
        int          _mode=0;
};

#endif // VR_KIT_H
