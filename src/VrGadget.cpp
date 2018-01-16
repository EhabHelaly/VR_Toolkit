#include "network.h"
#include "VrGadget.h"
#include <windows.h>
#include <sstream>

//#define SHOW_CONSOLE_MESSAGES
#define THREAD_DELAY_TIME 5

using namespace std;

static DWORD receiveFunction(LPVOID pointer);
VrGadget::VrGadget()
{

}
bool VrGadget::init(int Port)
{
    port=Port;
    _threadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) receiveFunction,this,0,&_threadID);
    return true;
}
bool VrGadget::release()
{
    _stopThread=true;
    WaitForMultipleObjects(1, &_threadHandle, TRUE, INFINITE);

    // Close all thread handles
    CloseHandle(_threadHandle);
    return true;
}

DWORD VrGadget::UDPthread()
{
    UDPsocket esp;
    esp.initReceiver(port);

    if (port==PORT_ADDRESS_ESP)//esp
    {
        UDPsocket espTarget;
        if (espTarget.initSender(PORT_ADDRESS_ESP,"VRGadget1."))
            espTarget.transmit("any_message",15);
    }

    while (!_stopThread )
    {
        if (esp.receive(_data.buffer,_size))
        {
            if (_size==PACKET_LENGTH)
                _parseDataESP();
            else if (_size==1)
                _parseDataBtn();
            else
                _parseDataApp();

        }
        Sleep(THREAD_DELAY_TIME);
    }

    esp.release();
    return 0;
}
void VrGadget::_parseDataESP()
{
    _sizeOld+=_size;
    if (!_size) // NO NEW DATA
        return;
#ifdef SHOW_CONSOLE_MESSAGES
    cout<<"Size: "<<_size<<"\tTotal: "<<_sizeOld<<endl;//*/
#endif // SHOW_CONSOLE_MESSAGES
    if (_sizeOld==PACKET_LENGTH && _data.buffer[PACKET_LENGTH-1]=='\n')
    {
        _R.x=_data.number[1];      // pitch
        _R.y=_data.number[2];         // yaw
        _R.z=_data.number[0];         // roll
        _R*=180/3.14159; // rad to degree
        _b  =_data.buffer[PACKET_LENGTH-2];
#ifdef SHOW_CONSOLE_MESSAGES
        std::cout<<_R.x<<" "<<_R.y<<" "<<_R.z<<" "<<(int)_b<<" "<<endl;
#endif // SHOW_CONSOLE_MESSAGES
        _sizeOld=0;
    }
    else if (_data.buffer[_sizeOld-1]=='\n' || _sizeOld>=PACKET_LENGTH)
        _sizeOld=0;

}
void VrGadget::_parseDataBtn()
{
    switch (_data.buffer[0])
    {
        case 'k':{_b=1; break;}
        case 'b':{_b=2; break;}
        case 'm':{_b=4; break;}
        case 'K':{_b=0; break;}
        case 'B':{_b=0; break;}
        case 'M':{_b=0; break;}
        default :{_b=0; break;}
    }
}
void VrGadget::_parseDataApp()
{
    float number[3];
    char c;
    int code=0;
    std::stringstream s(_data.buffer);
#ifdef SHOW_CONSOLE_MESSAGES
    std::cout<<_data.buffer<<endl;
#endif // SHOW_CONSOLE_MESSAGES
    _data.buffer[_size]=0;
    s>>number[0]>>c;
    int counter=0;
    while(!s.eof())
    {
        s>>code>>c;
        for (int n=0;n<3;n++)
            s>>number[n]>>c;
        if (counter++>100)
            return;
    }
    if (code!=81)// orientation
        return;

    if (port==PORT_ADDRESS_APP_HAND)
    {
        _R.x=number[1];
        _R.y=number[0];
        _R.z=number[2];
    }
    else // PORT_ADDRESS_APP_HEAD
    {
        _R.x=number[2]-90;
        _R.y=number[0]+90;
        _R.z=number[1]*-1;
        if (_R.z<= -90 || _R.z>= 90 )
        {
            _R.z=glm::sign(_R.z) * (180-glm::abs(_R.z) );
        }
        else
        {
            _R.x*=-1;
        }
    }

#ifdef SHOW_CONSOLE_MESSAGES
    std::cout<<_R.x<<" "<<_R.y<<" "<<_R.z<<" "<<endl;
#endif // SHOW_CONSOLE_MESSAGES

}
bool VrGadget::getRotation()
{
    ROLD=R;
    R=_R;
    DR=R-ROLD;
    // buttons
    for(int b=0; b<BUTTONS_NUMBER; b++)
    {
        buttonsOld[b]=buttons[b];
        buttons[b]= _b & (1<<b);
    }

    return true;
}
static DWORD receiveFunction(LPVOID pointer)
{
    return( (VrGadget*)pointer )->UDPthread();
}

