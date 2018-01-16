#include "VrKit.h"

VrKit::VrKit()
{

}
bool VrKit::init(int mode)
{
    _mode=mode;
    bool result=true;
    if (networkInit())
    {
        if (_mode&VR_POSITION_TRACKING)
        {
            result = result&&_headset.init();
        }
        if (_mode&VR_ORIENTATION_TRACKING)
        {
            result = result&&_headGadget.init(PORT_ADDRESS_APP_HEAD)
                           &&_handGadget.init(PORT_ADDRESS_APP_HAND);
        }
    }
    else
    {
        result=false;
    }

    return result;
}
void VrKit::getNewData()
{
    if (_mode&VR_POSITION_TRACKING)
    {
        _headset.getPosition();
    }
    if (_mode&VR_ORIENTATION_TRACKING)
    {
        _headGadget.getRotation();
        _handGadget.getRotation();
    }

}
void VrKit::release()
{
    if (_mode&VR_POSITION_TRACKING)
    {
        _headset.release();
    }
    if (_mode&VR_ORIENTATION_TRACKING)
    {
        _headGadget.release();
        _handGadget.release();
    }
    networkRelease();

}
// getter
glm::vec3 VrKit::getHeadPos()
{
    if (_mode&VR_POSITION_TRACKING)
    {
        return _headset.pos1New;
    }
    else
    {
        return glm::vec3(0,10,0);
    }
}
glm::vec3 VrKit::getHeadPosD()
{
    return _headset.pos1Delta;
}
glm::vec3 VrKit::getHandPos()
{
    if (_mode&VR_POSITION_TRACKING)
    {
        return _headset.pos2New;
    }
    else
    {
        return glm::vec3(0,10,0);
    }
}
glm::vec3 VrKit::getHandPosD()
{
    return _headset.pos2Delta;
}
glm::vec3 VrKit::getHeadRot()
{
    return _headGadget.R;
}
glm::vec3 VrKit::getHeadRotD()
{
    return _headGadget.DR;
}

glm::vec3 VrKit::getHandRot()
{
    return _handGadget.R;
}
glm::vec3 VrKit::getHandRotD()
{
    return _handGadget.DR;
}
bool VrKit::buttonIsDown(int b)
{
    return _handGadget.buttons[b]>0 && _handGadget.buttonsOld[b]==0;
}
bool VrKit::buttonIsOn(int b)
{
    return _handGadget.buttons[b]>0 && _handGadget.buttonsOld[b]>0;
}
bool VrKit::buttonIsUp(int b)
{
    return _handGadget.buttons[b]==0 && _handGadget.buttonsOld[b]>0;
}
bool VrKit::buttonIsOff(int b)
{
    return _handGadget.buttons[b]==0 && _handGadget.buttonsOld[b]==0;
}
