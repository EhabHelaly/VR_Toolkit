#ifndef VR_GADGET_H
#define VR_GADGET_G

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <windows.h>

#define BUTTONS_NUMBER 3
#define PACKET_LENGTH 14

#define PORT_ADDRESS_ESP        80
#define PORT_ADDRESS_APP_HAND 5556
#define PORT_ADDRESS_APP_HEAD 5555

class VrGadget{
public:
    VrGadget();
    bool init(int Port);
    bool release();
    bool getRotation();

    // thread function
    DWORD UDPthread();


    glm::vec3 R=glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 ROLD=glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 DR;

    int buttonsOld[BUTTONS_NUMBER]={0,0,0};
    int buttons   [BUTTONS_NUMBER]={0,0,0};


private:
    int port;
    glm::vec3 _R;
    uint8_t _b=0;
    union Data{
        char buffer[1024];
        float  number[3];
    }_data;
    int _size   =0;
    int _sizeOld=0;



    bool           _first=true;
    bool      _stopThread=false;
    bool _threadIsRunning=false;

    HANDLE  _threadHandle;
    DWORD   _threadID;
    void _parseDataESP();
    void _parseDataApp();
    void _parseDataBtn();
};

#endif // VR_GADGET_H
