#include <iostream>
#include "Scene.h"

using namespace std;

int main(int argc, char** argv)
{
    cout<<"-------------------------------------------------------------"<<endl;
    cout<<"------------------         VR App         -------------------"<<endl;
    cout<<"-------------------------------------------------------------"<<endl;

    Scene scene;
    scene.run();
    scene.release();
    return 0;
}
