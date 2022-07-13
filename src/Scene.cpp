#include "Scene.h"

#include <fstream>
#include <vector>
#include <string>
#include <iostream>
#include <vector>

#include <windows.h>

#include <GL/glew.h>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/intersect.hpp>

using namespace std;
using namespace glm;

int modelLoadingThread(void* world);

float exponent=5;

Scene::Scene()
{
    // Window settings
    _window.create("GameEngine", 1360, 760, 0);
    //_window.set(GE_WINDOW_FPS,20);

    //initialize shader program
    _shader.compileShaders("Data/colorShading.vert", "Data/colorShading.frag");
    _shader.addAttribute("vertexP");
    _shader.addAttribute("vertexUV");
    _shader.addAttribute("vertexN");
    _shader.addAttribute("vertexT");
    _shader.addAttribute("vertexB");
    _shader.linkShaders();

    // Camera settings
    _camera.init(_window.get(GE_WINDOW_WIDTH), _window.get(GE_WINDOW_HEIGHT));
    _camera.translateTo(glm::vec3(0,10,0));

    // VR App
    #ifdef VR_APP_N_KIT_ACTIVE
    initialzeVR();
    #endif // VR_APP_N_KIT_ACTIVE

    // Ray
    newModel()->load("Models/VR_icons/ray.obj"); //hard coded to be fixed later
    _models[0]->shown=false;

    // Scene settings
    newModel()->load("Models/Background/hdri.obj");
    newModel()->load("Models/Background/plane.obj");
    ge::translate(_models[2], &_camera, glm::vec3(0,-0.5,0),GE_PERSPECTIVE_WORLD);
}
bool Scene::isActive()
{
    return _appState==AppState::PLAY;
}

Model* Scene::newModel()
{
    _models.push_back(new Model());
    return _models.back();
}
Model* Scene::newModel2D()
{
    _models_2D.push_back(new Model());
    return _models_2D.back();
}

void Scene::delModel(Model* model)
{
    for (unsigned int m=0; m<_models.size(); m++)
    {
        if (_models[m]==model)
        {
            delete model;
            _models.erase(_models.begin()+m);
            return;
        }
    }
    for (unsigned int m=0; m<_models_2D.size(); m++)
    {
        if (_models_2D[m]==model)
        {
            delete model;
            _models_2D.erase(_models_2D.begin()+m);
            return;
        }
    }
}
void Scene::run()
{
    loadScene("Scenes/main.scene");
    startLoadingThread();
    while(isActive())
    {
        processInputSDL();
        #ifdef VR_APP_N_KIT_ACTIVE
        processInputVR();
        #endif // VR_APP_N_KIT_ACTIVE
        render();
    }
    saveScene("Scenes/main.scene");
}
void Scene::release()
{
    _window.release();

    closeLoadingThread();

    while (_models.size())
        delModel(_models[0]);

    while (_models_2D.size())
        delModel(_models_2D[0]);


    #ifdef VR_APP_N_KIT_ACTIVE
    // VR App
    _vrKit.release();
    #endif // VR_APP_N_KIT_ACTIVE

}
void Scene::processInputSDL()
{
    SDL_Event evnt;

    glm::vec3 _plusHeadPos(0,0,0);
    glm::vec3 _plusHeadRot(0,0,0);
    float speed=_window.get(GE_WINDOW_TIME_DELTA)/50;

    while (SDL_PollEvent(&evnt))
    {
       switch (evnt.type)
       {
            case SDL_QUIT:
                _appState = AppState::EXIT;
                break;
            case SDL_KEYDOWN:
                switch(evnt.key.keysym.sym)
                {
                    //Active model control
                    //Translation
                    case SDLK_1:
                        {
                             static unsigned int n=3;
                            _activeModel = _models[n];
                            if (n < _models.size()-1){ n++;}
                            else {n=3;}
                            break;
                        }
                    case SDLK_2:
                    {
                        std::string fileName=getFileName("obj\0*.obj\0");
                        if (fileName.length())
                        {
                            newModel()->modelFileName=fileName;
                        }
                        break;
                    }
                    case SDLK_3:
                        delModel(_activeModel);
                        _activeModel=0;
                        break;
                    case SDLK_4:
                    {
                        std::string fileName=getFileName("obj\0*.obj\0");
                        if (fileName.length())
                        {
                            newModel2D()->load(fileName);

                            ge::translate(_models_2D.back(),&_camera,glm::vec3(0,0,-0.2),GE_PERSPECTIVE_WORLD);
                        }
                        break;
                    }
                    case SDLK_5:
                    {
                        std::string fileName=getFileName("obj\0*.obj\0");
                        if (fileName.length())
                        {
                            std::string fileName2=getFileName("mtl\0*.mtl\0");
                            Model* model=newModel();
                            model->materialFileName=fileName2;
                            model->modelFileName=fileName;
                        }
                        break;
                    }
                    case SDLK_6:
                    {
                        if (_activeModel)
                        {
                            std::string fileName2=getFileName("mtl\0*.mtl\0");
                            _activeModel->updateMaterials(fileName2);
                        }
                        break;
                    }
                    case SDLK_7:
                    {
                        exponent+=1;
                        break;
                    }
                    case SDLK_8:
                    {
                        exponent-=1;
                        break;
                    }

                }
       }
    }
    const uint8_t* keys=SDL_GetKeyboardState(0);
    // Speed control
    if (keys[SDL_SCANCODE_LSHIFT])    {speed/=10;}
    // translation
    if (keys[SDL_SCANCODE_W])    {_plusHeadPos.z =-speed;}
    if (keys[SDL_SCANCODE_S])    {_plusHeadPos.z = speed;}
    if (keys[SDL_SCANCODE_A])    {_plusHeadPos.x =-speed;}
    if (keys[SDL_SCANCODE_D])    {_plusHeadPos.x = speed;}
    if (keys[SDL_SCANCODE_O])    {_plusHeadPos.y = speed;}
    if (keys[SDL_SCANCODE_P])    {_plusHeadPos.y =-speed;}
    // rotation
    if (keys[SDL_SCANCODE_UP])   {_plusHeadRot.x =-speed*2.5;}
    if (keys[SDL_SCANCODE_DOWN]) {_plusHeadRot.x = speed*2.5;}
    if (keys[SDL_SCANCODE_RIGHT]){_plusHeadRot.y = speed*2.5;}
    if (keys[SDL_SCANCODE_LEFT]) {_plusHeadRot.y =-speed*2.5;}
    if (keys[SDL_SCANCODE_9])    {_plusHeadRot.z =-speed*2.5;}
    if (keys[SDL_SCANCODE_0])    {_plusHeadRot.z = speed*2.5;}

    if (_activeModel)
    {
        // Translation
        if (keys[SDL_SCANCODE_T]) {ge::translate(_activeModel, &_camera, glm::vec3(0,0,-speed/2)); }
        if (keys[SDL_SCANCODE_G]) {ge::translate(_activeModel, &_camera, glm::vec3(0,0,speed/2));  }
        if (keys[SDL_SCANCODE_F]) {ge::translate(_activeModel, &_camera, glm::vec3(-speed/2,0,0)); }
        if (keys[SDL_SCANCODE_H]) {ge::translate(_activeModel, &_camera, glm::vec3(speed/2,0,0));  }
        if (keys[SDL_SCANCODE_C]) {ge::translate(_activeModel, &_camera, glm::vec3(0,speed/2,0));  }
        if (keys[SDL_SCANCODE_V]) {ge::translate(_activeModel, &_camera, glm::vec3(0,-speed/2,0)); }
        // Rotation
        if (keys[SDL_SCANCODE_I]) {ge::rotate(_activeModel, &_camera, glm::vec3(speed*2, 0, 0)); }
        if (keys[SDL_SCANCODE_K]) {ge::rotate(_activeModel, &_camera, glm::vec3(-speed*2, 0, 0));}
        if (keys[SDL_SCANCODE_J]) {ge::rotate(_activeModel, &_camera, glm::vec3(0, 0, speed*2)); }
        if (keys[SDL_SCANCODE_L]) {ge::rotate(_activeModel, &_camera, glm::vec3(0, 0, -speed*2));}
        if (keys[SDL_SCANCODE_N]) {ge::rotate(_activeModel, &_camera, glm::vec3(0, speed*2, 0)); }
        if (keys[SDL_SCANCODE_M]) {ge::rotate(_activeModel, &_camera, glm::vec3(0, -speed*2, 0));}
        // Scaling
        if (keys[SDL_SCANCODE_Z]) {ge::scale(_activeModel, &_camera, glm::vec3(1.02,1.02,1.02));  }
        if (keys[SDL_SCANCODE_X]) {ge::scale(_activeModel, &_camera, glm::vec3(1/1.02,1/1.02,1/1.02));}
    }

    // lights
    if (keys[SDL_SCANCODE_KP_1]) {for (int s=0;s<5;s++) _lights[s].power /=1.1;}
    if (keys[SDL_SCANCODE_KP_2]) {for (int s=0;s<5;s++) _lights[s].power *=1.1;}
    if (keys[SDL_SCANCODE_KP_3]) {}
    if (keys[SDL_SCANCODE_KP_4]) {for (int s=0;s<5;s++) _lights[s].position.x /=1.01;}
    if (keys[SDL_SCANCODE_KP_5]) {for (int s=0;s<5;s++) _lights[s].position.z /=1.01;}
    if (keys[SDL_SCANCODE_KP_6]) {for (int s=0;s<5;s++) _lights[s].position.x *=1.01;}
    if (keys[SDL_SCANCODE_KP_7]) {for (int s=0;s<5;s++) _lights[s].position.y /=1.01;}
    if (keys[SDL_SCANCODE_KP_8]) {for (int s=0;s<5;s++) _lights[s].position.z *=1.01;}
    if (keys[SDL_SCANCODE_KP_9]) {for (int s=0;s<5;s++) _lights[s].position.y *=1.01;}

    // MOUSE
    static int mxO=0,myO=0;
    int mx=0,my=0;
    int state=SDL_GetMouseState(&mx,&my);
    if (state & SDL_BUTTON(SDL_BUTTON_RIGHT))
    {
        _plusHeadRot.y+=(mx-mxO)/2;
        _plusHeadRot.x+=(my-myO)/2;
    }

    if (state & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        _activeModel=getTargetModel();
    }
    mxO=mx;
    myO=my;

    // Camera
    _camera.rotate(_plusHeadRot);
    _camera.translate(_plusHeadPos,GE_PERSPECTIVE_CAMERA);

    // HDRI
    ge::translateTo(_models[1],&_camera,_camera.position,GE_PERSPECTIVE_WORLD);
    ge::translate  (_models[1],&_camera,glm::vec3(0,-15,0),GE_PERSPECTIVE_WORLD);

    //Ray
    #ifndef VR_APP_N_KIT_ACTIVE
    glm::vec3 mouse_diviations=-1.0f*glm::vec3(
                                               (my-_camera.height/2.0f)*_camera.fovy/_camera.height,
                                               (mx-_camera.width /2.0f)*_camera.fovy/_camera.height,
                                               0);
    ge::translateTo(_models[0],&_camera,_camera.position,GE_PERSPECTIVE_WORLD);
    ge::rotateTo   (_models[0],&_camera,-1.0f*_camera.orientation+mouse_diviations);
    #endif // VR_APP_N_KIT_ACTIVE
}
#ifdef VR_APP_N_KIT_ACTIVE
void Scene::initialzeVR()
{
    if (!_vrKit.init(VR_POSITION_TRACKING|VR_ORIENTATION_TRACKING))
    {
        cout<<"VR Kit failed to start"<<endl;
    }//*/
    _vrApp.loadCategorizedModels();
    newModel2D()->load("Models/VR_icons/icon.obj");
    newModel2D()->load("Models/VR_icons/icon.obj","Models/VR_icons/loading.mtl");//*/
    newModel2D()->load("Models/VR_icons/icon.obj","Models/VR_icons/forward.mtl");//*/
    ge::translate(_models_2D[0],&_camera,glm::vec3(0,  0,-0.2),GE_PERSPECTIVE_WORLD);
    ge::translate(_models_2D[1],&_camera,glm::vec3(0,-0.35,-1),GE_PERSPECTIVE_WORLD);
    ge::translate(_models_2D[2],&_camera,glm::vec3(0,-0.35,-1),GE_PERSPECTIVE_WORLD);

}
void Scene::processInputVR()
{
    int id=0;
    static glm::vec3 posShift(0,0,0);
    _vrKit.getNewData();

    /// Menu control
    if (_vrKit.buttonIsDown(0))
    {
        _vrApp.cursor=0;
        if (_vrApp.level>0) // menu opened?
            _vrApp.level++; // means select menu option
    }
    if (_vrKit.buttonIsDown(1))
    {
        _vrApp.cursor=0;
        if (_vrApp.level>0) // menu opened?
            _vrApp.level--; // means go back in menu or close menu
        else
            _vrApp.level=1; // means open menu
    }
    if (glm::abs(_vrKit.getHandRot().z)>=10 )
    {
        _vrApp.cursor-=_vrKit.getHandRot().z/20;
    }
    /// Menu events
    switch (_vrApp.level) // menu levels
    {
    case 1: // Main Menu
        {
            _vrApp.choice[1]=(int)(glm::abs(_vrApp.cursor)/MENU_ANGLES_PER_CHOICE)%MENU_OPTIONS_MAIN_SIZE;
            if (_vrApp.cursor<0)
            {
                _vrApp.choice[1]=MENU_OPTIONS_MAIN_SIZE-_vrApp.choice[1]-1;
            }
            id=loadMap(_vrApp.imPath+ _vrApp.imMain[_vrApp.choice[1]]);
            deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
            _models_2D[VR_M_MENU]->materials[0]->mAmbient=id;
            id=loadMap(_vrApp.imPath+ _vrApp.imMain[_vrApp.choice[1]]);
            deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
            _models_2D[VR_M_MENU]->materials[0]->mDepth=id;
            break;
        }
    case 2: // Sub Menu for (x,y,z) or models images
        {
            switch ( _vrApp.choice[1] )
            {
                case MENU_ADD:
                {
                    _vrApp.choice[2] =(int)(glm::abs(_vrApp.cursor)/MENU_ANGLES_PER_CHOICE)%MENU_OPTIONS_ADD_SIZE;
                    if (_vrApp.cursor<0)
                    {
                        _vrApp.choice[2]=MENU_OPTIONS_ADD_SIZE-_vrApp.choice[2]-1;
                    }
                    id=loadMap(_vrApp.imPath+ _vrApp.imAdd[_vrApp.choice[2]]);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
                    _models_2D[VR_M_MENU]->materials[0]->mAmbient=id;
                    id=loadMap(_vrApp.imPath+ _vrApp.imAdd[_vrApp.choice[2]]);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
                    _models_2D[VR_M_MENU]->materials[0]->mDepth=id;
                    break;
                }
                case MENU_EDIT:
                {
                    _vrApp.choice[2] =(int)(glm::abs(_vrApp.cursor)/MENU_ANGLES_PER_CHOICE)%MENU_OPTIONS_EDIT_SIZE;
                    if (_vrApp.cursor<0)
                    {
                        _vrApp.choice[2]=MENU_OPTIONS_EDIT_SIZE-_vrApp.choice[2]-1;
                    }
                    id=loadMap(_vrApp.imPath+ _vrApp.imEdit[_vrApp.choice[2]]);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
                    _models_2D[VR_M_MENU]->materials[0]->mAmbient=id;
                    id=loadMap(_vrApp.imPath+ _vrApp.imEdit[_vrApp.choice[2]]);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
                    _models_2D[VR_M_MENU]->materials[0]->mDepth=id;
                    break;
                }
                case MENU_DELETE:
                {
                    _vrApp.level=-1; // editing mode
                    break;
                }
                case MENU_MOVE:
                case MENU_ROTATE:
                case MENU_SCALE:
                {
                    _vrApp.choice[2]=(int)(glm::abs(_vrApp.cursor)/MENU_ANGLES_PER_CHOICE)%MENU_SUBCHOICES;
                    if (_vrApp.cursor<0)
                    {
                        _vrApp.choice[2]=MENU_SUBCHOICES-_vrApp.choice[2]-1;
                    }
                    id=loadMap(_vrApp.imPath+ _vrApp.imTrans[ (_vrApp.choice[1]-MENU_MOVE)*MENU_SUBCHOICES +_vrApp.choice[2]] );
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
                    _models_2D[VR_M_MENU]->materials[0]->mAmbient=id;
                    id=loadMap(_vrApp.imPath+ _vrApp.imTrans[ (_vrApp.choice[1]-MENU_MOVE)*MENU_SUBCHOICES +_vrApp.choice[2]] );
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
                    _models_2D[VR_M_MENU]->materials[0]->mDepth=id;
                    break;
                }
            }
            break;
        }
    case 3:
        {
            switch (_vrApp.choice[1])
            {
            case MENU_ADD:
                {
                    _vrApp.choice[3] =(int)  (glm::abs(_vrApp.cursor)/MENU_ANGLES_PER_CHOICE)%(_vrApp.categories[_vrApp.choice[2]].models.size());
                    if (_vrApp.cursor<0)
                    {
                        _vrApp.choice[3]=(_vrApp.categories[_vrApp.choice[2]].models.size() )-_vrApp.choice[3]-1;
                    }
                    id=loadMap(_vrApp.getFileName(_vrApp.choice[2],_vrApp.choice[3],1) + ".jpg" );
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
                    _models_2D[VR_M_MENU]->materials[0]->mAmbient= id;
                    id=loadMap(_vrApp.imPath+ _vrApp.imModelMask);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
                    _models_2D[VR_M_MENU]->materials[0]->mDepth  = id;
                    break;
                }
            case MENU_EDIT:
            case MENU_DELETE : // not reachable
            case MENU_MOVE:
            case MENU_ROTATE:
            case MENU_SCALE:
                {
                    _vrApp.level=-1; // editing mode
                    break;
                }
            }
            break;
        }
    case 4:
        {
            switch (_vrApp.choice[1])
            {
            case MENU_ADD:
                {
                    _vrApp.choice[4] =(int)((glm::abs(_vrApp.cursor))/MENU_ANGLES_PER_CHOICE)%_vrApp.categories[_vrApp.choice[2]].models[_vrApp.choice[3]];
                    if (_vrApp.cursor<0)
                    {
                        _vrApp.choice[4]=( _vrApp.categories[_vrApp.choice[2]].models[_vrApp.choice[3]] )-_vrApp.choice[4]-1;
                    }
                    id=loadMap(_vrApp.getFileName(_vrApp.choice[2],_vrApp.choice[3],_vrApp.choice[4]+1) + ".jpg" );
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mAmbient);
                    _models_2D[VR_M_MENU]->materials[0]->mAmbient= id;
                    id=loadMap(_vrApp.imPath+ _vrApp.imModelMask);
                    deleteMap(_models_2D[VR_M_MENU]->materials[0]->mDepth);
                    _models_2D[VR_M_MENU]->materials[0]->mDepth  = id;
                    break;
                }
            }
            break;
        }
    case 5:
        {
            switch (_vrApp.choice[1])
            {
            case MENU_ADD:
                {
                    string mdl_file_name=_vrApp.getFileName(_vrApp.choice[2],_vrApp.choice[3],0                )+".obj";
                    string mtl_file_name=_vrApp.getFileName(_vrApp.choice[2],_vrApp.choice[3],_vrApp.choice[4]+1)+".mtl";
                    Model* model=newModel();
                    model->materialFileName=mtl_file_name;
                    model->modelFileName   =mdl_file_name;
                    ge::translateTo(model,&_camera,_camera.position ,GE_PERSPECTIVE_WORLD);
                    ge::translate  (model,&_camera,glm::vec3(0,-1.0f*_camera.position.y,-15),GE_PERSPECTIVE_CAMERA);

                    _vrApp.level=0;// close menu
                    break;
                }
            }
            break;
        }
    case -1: // editing mode (menu is hidden and arrow is shown)
        {
            if (_vrKit.buttonIsOff(0)) // no commands yet
            {
                getTargetModel();
                break;
            }
            else if (_vrKit.buttonIsUp(0))
            {
                _activeModel=0;
                break;
            }
            else if (_vrKit.buttonIsDown(0)) // bind model to be active one
            {
                _activeModel=getTargetModel();
                _targetModel=0;
                break;
            }
            else /// buttonIsOn
            {
                if (!_activeModel)
                {
                    break;
                }
                switch (_vrApp.choice[1])
                {
                case MENU_EDIT:
                    {
                        switch (_vrApp.choice[2])
                        {
                        case MENU_EDI_TEXTURE:
                            {
                                int id_start=_activeModel->materialFileName.find_last_of("_")+1;
                                int id_end  =_activeModel->materialFileName.find_last_of(".");
                                string mat  =_activeModel->materialFileName.substr(0,id_start);
                                int mat_id  =atoi(_activeModel->materialFileName.substr(id_start,id_end).c_str());
                                if (!_activeModel->updateMaterials(mat+to_string(++mat_id)+".mtl")  )
                                {
                                    _activeModel->updateMaterials(mat+"1.mtl");
                                }
                                _activeModel=0;
                                break;
                            }
                        case MENU_EDIT_CLONE:
                            {
                                Model* model=newModel();
                                model->materialFileName=_activeModel->materialFileName;
                                model->modelFileName   =_activeModel->modelFileName;
                                ge::translateTo(model,&_camera,_camera.position ,GE_PERSPECTIVE_WORLD);
                                ge::translate  (model,&_camera,glm::vec3(0,-1.0f*_camera.position.y,-10),GE_PERSPECTIVE_CAMERA);
                                ge::scaleTo    (model,&_camera,_activeModel->scaling,GE_PERSPECTIVE_OBJECT);
                                _activeModel=0;
                                break;
                            }
                        }
                        break;
                    }
                case MENU_DELETE:
                    {
                            delModel(_activeModel);
                            _activeModel=0;
                    }
                case MENU_MOVE:
                case MENU_ROTATE:
                case MENU_SCALE:
                    {
                        glm::vec3 value(0,0,0);
                        switch (_vrApp.choice[1])
                        {
                            case MENU_MOVE  :
                            {
                                switch ( _vrApp.choice[2] )
                                {
                                    case MENU_TRANSFORM_X:
                                    case MENU_TRANSFORM_Y:
                                    case MENU_TRANSFORM_Z:
                                    {
                                        value[_vrApp.choice[2]] = 0.05f*_vrKit.getHandRot().z ;
                                        ge::translate(_activeModel,&_camera,value,GE_PERSPECTIVE_WORLD );
                                        break;
                                    }
                                    case MENU_TRANSFORM_XYZ:
                                    {
                                        value.z =-10- 0.05f*_vrKit.getHandRot().x; break;
                                        ge::translateTo(_activeModel,&_camera,_camera.position,GE_PERSPECTIVE_WORLD );
                                        ge::translate  (_activeModel,&_camera,value           ,GE_PERSPECTIVE_CAMERA);
                                        break;
                                    }
                                }
                                break;
                            }
                            case MENU_ROTATE:
                            {
                                switch ( _vrApp.choice[2] )
                                {
                                    case MENU_TRANSFORM_X:
                                    case MENU_TRANSFORM_Y:
                                    case MENU_TRANSFORM_Z:
                                    {
                                        value[_vrApp.choice[2]] = 0.05f*_vrKit.getHandRot().z ;
                                        ge::rotate(_activeModel,&_camera,value,GE_PERSPECTIVE_OBJECT); break;
                                        break;
                                    }
                                    case MENU_TRANSFORM_XYZ:
                                    {
                                        ge::rotateTo(_activeModel,&_camera,_vrKit.getHandRot(),GE_PERSPECTIVE_OBJECT);
                                        break;
                                    }
                                }
                                break;
                            }
                            case MENU_SCALE :
                            {
                                value=glm::vec3(1,1,1);
                                switch ( _vrApp.choice[2] )
                                {
                                    case MENU_TRANSFORM_X:
                                    case MENU_TRANSFORM_Y:
                                    case MENU_TRANSFORM_Z:
                                    {

                                        value[_vrApp.choice[2]] += 0.05f*_vrKit.getHandRot().z ;
                                        ge::scale(_activeModel,&_camera,value,GE_PERSPECTIVE_OBJECT);
                                        break;
                                    }
                                    case MENU_TRANSFORM_XYZ:
                                    {
                                        value+=glm::vec3(0.0001*_vrKit.getHandRot().z,0.0001*_vrKit.getHandRot().z,0.0001*_vrKit.getHandRot().z);
                                        ge::scale(_activeModel,&_camera,value ,GE_PERSPECTIVE_OBJECT);
                                        break;
                                    }
                                }
                                break;
                            }
                        }

                        break;
                    }
                }
            }
        }
    }

    // Head
    _camera.rotateTo(_vrKit.getHeadRot());
    const float f=1.0;
    if (_vrKit.buttonIsOn(2))
    {
        _camera.translate(glm::vec3(0,0,-1),GE_PERSPECTIVE_CAMERA);
    }
    else if (_vrKit.buttonIsUp(2))
    {
        posShift.x=_camera.position.x-f*_vrKit.getHeadPos().x;
        posShift.z=_camera.position.z-f*_vrKit.getHeadPos().z;
    }
    else
    {
        _camera.translateTo(glm::vec3(f,f,f) *_vrKit.getHeadPos()+posShift,GE_PERSPECTIVE_WORLD);
    }

    // Ray
    _models[0]->shown=(_vrApp.level==-1);
    ge::rotateTo   (_models[0],&_camera,-1.0f*_vrKit.getHandRot(),GE_PERSPECTIVE_WORLD);
    //ge::translateTo(_models[0],&_camera, glm::vec3(f,1,f) *_vrKit.getHandPos(),GE_PERSPECTIVE_WORLD);
    ge::translateTo(_models[0],&_camera, _camera.position,GE_PERSPECTIVE_WORLD);
    ge::translate  (_models[0],&_camera, glm::vec3(-0.5,-0.5,-0.5),GE_PERSPECTIVE_CAMERA);

    // Menu
    _models_2D[VR_M_MENU   ]->shown=_vrApp.level>0;

    // loading icon
    _models_2D[VR_M_LOADING]->shown=_loadingStatus;
    ge::rotate(_models_2D[VR_M_LOADING],&_camera,glm::vec3(0,0,-5),GE_PERSPECTIVE_OBJECT);

    // forward icon
    _models_2D[VR_M_FORWARD]->shown=_vrKit.buttonIsOn(2);

}
#endif // VR_APP_N_KIT_ACTIVE
bool Scene::loadScene(std::string sceneFileName)
{
	ifstream  sceneFile;
	std::string    line;
    Model* 	    model=0;
    Light* 	    light=0;
    int            s=-1;// no light source

	sceneFile.open(sceneFileName.c_str());
	if (!sceneFile.is_open())
	{
		std::cout<<sceneFileName.c_str()<<" Not found or could not be opened\n";
		return false;
	}

    while (getline(sceneFile,line))
    {
        stringstream stream(line);	// converts each line to string stream to be scanned word by word
        string                tag;	// string holding scanned words   from string stream
        float 		     f1,f2,f3;	// floats holding scanned numbers from string stream

        stream>>tag;				// get first word in line

        if (!tag.compare("load"))
        {
            model=newModel();
            model->modelFileName=line.substr(line.find_first_of(" ")+1);
        }
        if (!tag.compare("load2D"))
        {
            model=newModel2D();
            model->modelFileName=line.substr(line.find_first_of(" ")+1);
            model->load();
        }
        else if (!tag.compare("material"))
        {
            if (model)
            {
                model->materialFileName=line.substr(line.find_first_of(" ")+1);
            }
        }
        else if (!tag.compare("move"))
        {
            if (model)
            {
                stream>>f1>>f2>>f3;
                ge::translateTo(model, &_camera, glm::vec3(f1,f2,f3),GE_PERSPECTIVE_WORLD);
            }
        }
        else if (!tag.compare("rotate"))
        {
            if (model)
            {
                stream>>f1>>f2>>f3;
                ge::rotateTo(model, &_camera, glm::vec3(f1,f2,f3),GE_PERSPECTIVE_WORLD);
            }
        }
        else if (!tag.compare("scale"))
        {
            if (model)
            {
                stream>>f1>>f2>>f3;
                ge::scaleTo(model, &_camera, glm::vec3(f1,f2,f3),GE_PERSPECTIVE_WORLD);
            }
        }
        else if (!tag.compare("light_type"))
        {
            s = (s<LIGHT_SOURCES_MAX-1)? s+1 : 0;
            light=_lights+s;
            stream>>light->type;
        }
        else if (!tag.compare("light_power"))
        {
            if (light)
            {
                stream>>light->power;
            }
        }
        else if (!tag.compare("light_position"))
        {
            if (light)
            {
                stream>>light->position.x>>light->position.y>>light->position.z;
            }
        }
        else if (!tag.compare("light_color"))
        {
            if (light)
            {
                stream>>light->color.x>>light->color.y>>light->color.z;
            }
        }
        else if (!tag.compare("light_direction"))
        {
            if (light)
            {
                stream>>light->direction.x>>light->direction.y>>light->direction.z;
            }
        }

    }
    sceneFile.close();
    return true;

}
bool Scene::saveScene(std::string sceneFileName)
{
    ofstream sceneFile;
    std::string  line;

    sceneFile.open(sceneFileName);
    if (!sceneFile.is_open())
	{
		std::cout<<"Failed to create or open "<<sceneFileName.c_str()<<endl;
		return false;
	}

	// Light sources
	for (unsigned int s=0;s<LIGHT_SOURCES_MAX;s++)
    {
        sceneFile<<"light_type "     <<_lights[s].type<<endl;
        sceneFile<<"light_power "    <<_lights[s].power<<endl;
        sceneFile<<"light_position " <<_lights[s].position.x<<" "<<_lights[s].position.y<<" "<<_lights[s].position.z<<" "<<endl;
        sceneFile<<"light_color "    <<_lights[s].color.x<<" "<<_lights[s].color.y<<" "<<_lights[s].color.z<<" "<<endl;
        sceneFile<<"light_direction "<<_lights[s].direction.x<<" "<<_lights[s].direction.y<<" "<<_lights[s].direction.z<<" "<<endl;
        sceneFile<<endl;
    }

    /*
    // 2D Models like UI icons
	for (unsigned int a=0;a<_models_2D.size();a++)
    {
        sceneFile<<"load2D "  <<_models_2D[a]->modelFileName<<endl;
        sceneFile<<"material "<<_models_2D[a]->materialFileName<<endl;
        sceneFile<<"move "    <<_models_2D[a]->position.x<<" "<<_models_2D[a]->position.y<<" "<<_models_2D[a]->position.z<<" "<<endl;
        sceneFile<<"rotate "  <<_models_2D[a]->orientation.x<<" "<<_models_2D[a]->orientation.y<<" "<<_models_2D[a]->orientation.z<<" "<<endl;
        sceneFile<<"scale "   <<_models_2D[a]->scaling.x<<" "<<_models_2D[a]->scaling.y<<" "<<_models_2D[a]->scaling.z<<" "<<endl;
        sceneFile<<endl;
    }//*/

    // 3D Models
	for (unsigned int a=_3D_MODELS_BEGIN;a<_models.size();a++)
    {
        sceneFile<<"load "    <<_models[a]->modelFileName<<endl;
        sceneFile<<"material "<<_models[a]->materialFileName<<endl;
        sceneFile<<"move "    <<_models[a]->position.x<<" "<<_models[a]->position.y<<" "<<_models[a]->position.z<<" "<<endl;
        sceneFile<<"rotate "  <<_models[a]->orientation.x<<" "<<_models[a]->orientation.y<<" "<<_models[a]->orientation.z<<" "<<endl;
        sceneFile<<"scale "   <<_models[a]->scaling.x<<" "<<_models[a]->scaling.y<<" "<<_models[a]->scaling.z<<" "<<endl;
        sceneFile<<endl;
    }
    sceneFile.close();
    return true;
}
void Scene::render()
{
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    _shader.use();

    /// Set GLSL program uniforms ///
    //  same for all Models in Scene
    _shader.setMat4("V", _camera.viewMatrix);
    _shader.setFloat("cosTime", cos(_window.get(GE_WINDOW_TIME)/500)/2+0.5);

    // light
    for(int s=0;s<LIGHT_SOURCES_MAX;s++)
    {
        string L="lights["+to_string(s) +"].";
        _shader.setVec3 (L+"direction_cameraspace", transpose(inverse(glm::mat3(_camera.viewMatrix))) *_lights[s].direction );
        _shader.setVec3 (L+"position_cameraspace" , vec3(_camera.viewMatrix * vec4(_lights[s].position,1)));
        _shader.setVec3 (L+"color"                , _lights[s].color);
        _shader.setFloat(L+"power"                , _lights[s].power);
        _shader.setInt  (L+"type"                 , _lights[s].type);
    }

    // render opaque objects
    glDepthMask(GL_TRUE);   // enables updating depth buffer
    glDepthFunc(GL_LEQUAL); // Accept fragment if it closer to the camera than the former one
    for (unsigned int i = 0; i < _models.size(); i++)
    {
        if (_models[i]->shown)
            renderModel(_models[i],false);
    }

    // Render objects with transparency
    glDepthMask(GL_FALSE);  // disables updating depth buffer
    glDepthFunc(GL_LEQUAL); // Accept fragment if it closer to the camera than the former one
    for (unsigned int i = 0; i < _models.size(); i++)
    {
        if (_models[i]->shown)
            renderModel(_models[i],true);
    }

    // Render 2D objects
    glDepthMask(GL_TRUE);   // enables updating depth buffer
    glDepthFunc(GL_ALWAYS); // Accepts all fragments
    for (unsigned int i = 0; i < _models_2D.size(); i++)
    {
        if (_models_2D[i]->shown)
            renderModel2D(_models_2D[i]);
    }

    _shader.unuse();

    _window.swapBuffer();
}
void Scene::renderModel(Model* model, bool transparency)
{
    /// Set GLSL program uniforms ///
    //  same for all Objects in Model
    glm::mat4 mvp   = _camera.getCameraMatrix() * model-> modelMatrix;
    glm::mat3 normalMatrix = transpose(inverse(glm::mat3(_camera.viewMatrix * model-> modelMatrix)));
    _shader.setMat4("MVP",mvp);
    _shader.setMat4("M",model-> modelMatrix);
    _shader.setMat3("N",normalMatrix);
    _shader.setInt("targetModel",model==_targetModel? 1:0);

    // draw each object in model
    for (vector<Object>::iterator obj=model->objects.begin();obj!=model->objects.end();obj++)
    {
        if ( (obj->material->depth!=1) ^ transparency)
            continue;

        // ambient map
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->material->mAmbient);
        _shader.setInt("maps.ambient",0);

        // diffuse map
        // Bind our diffuse in Texture Unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, obj->material->mDiffuse);
        _shader.setInt("maps.diffuse",1);

        // normal map
        // Bind our texture in Texture Unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, obj->material->mNormal);
        _shader.setInt("maps.normal",2);

        // specular map
        // Bind our texture in Texture Unit 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, obj->material->mSpecular);
        _shader.setInt("maps.specular",3);

        // emission map
        // Bind our texture in Texture Unit 4
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, obj->material->mEmission);
        _shader.setInt("maps.emission",4);

        // depth map
        // Bind our texture in Texture Unit 5
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, obj->material->mDepth);
        _shader.setInt("maps.depth",5);

        // Coefficients  (used in case no maps loaded)
        // ambient
        _shader.setVec3("mat.Ka",obj->material->ambient);
        // diffuse
        _shader.setVec3("mat.Kd",obj->material->diffuse);
        // gloss
        _shader.setVec3("mat.Ks",obj->material->specular);
        // emission
        _shader.setVec3("mat.Ke",obj->material->emission);
        // depth
        _shader.setFloat("mat.depth",obj->material->depth);
        // exponent
        _shader.setFloat("exp",exponent);

        obj->draw();
    }

}
void Scene::renderModel2D(Model* model)
{
    /// Set GLSL program uniforms ///
    //  same for all Objects in Model
    glm::mat4 mvp   = _camera.getProjectionMatrix() * model-> modelMatrix;
    _shader.setMat4("V", mat4(1.0f));
    _shader.setMat4("MVP",mvp);
    _shader.setMat4("M",model-> modelMatrix);
    _shader.setMat3("N",glm::mat3(model-> modelMatrix) );
    _shader.setInt("targetModel",0);

    // draw each object in model
    for (vector<Object>::iterator obj=model->objects.begin();obj!=model->objects.end();obj++)
    {
        // ambient map
        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, obj->material->mAmbient);
        _shader.setInt("maps.ambient",0);

        // diffuse map
        // Bind our diffuse in Texture Unit 1
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, obj->material->mDiffuse);
        _shader.setInt("maps.diffuse",1);

        // normal map
        // Bind our texture in Texture Unit 2
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, obj->material->mNormal);
        _shader.setInt("maps.normal",2);

        // specular map
        // Bind our texture in Texture Unit 3
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, obj->material->mSpecular);
        _shader.setInt("maps.specular",3);

        // emission map
        // Bind our texture in Texture Unit 4
        glActiveTexture(GL_TEXTURE4);
        glBindTexture(GL_TEXTURE_2D, obj->material->mEmission);
        _shader.setInt("maps.emission",4);

        // depth map
        // Bind our texture in Texture Unit 5
        glActiveTexture(GL_TEXTURE5);
        glBindTexture(GL_TEXTURE_2D, obj->material->mDepth);
        _shader.setInt("maps.depth",5);

        // Coefficients  (used in case no maps loaded)
        // ambient
        _shader.setVec3("mat.Ka",obj->material->ambient);
        // diffuse
        _shader.setVec3("mat.Kd",obj->material->diffuse);
        // gloss
        _shader.setVec3("mat.Ks",obj->material->specular);
        // emission
        _shader.setVec3("mat.Ke",obj->material->emission);
        // depth
        _shader.setFloat("mat.depth",obj->material->depth);

        obj->draw();
    }

}
int Scene::loadingThread()
{

    while(!_stopThread)
    {
        for (unsigned int a=0;a<_models.size();a++)
        {
            Model* model=_models[a];
            if (model->parsed==PARSING_PENDING)
            {
                _window.bindContext();
                _loadingStatus=true;

                if (model->load())
                {
                    //cout<<"loaded "<<model->modelFileName<<endl;
                }
                else
                {
                    cout<<"Failed to load 3D Model ->"<<model->modelFileName<<endl;
                }
                _window.unbindContext();
                _loadingStatus=false;
            }
        }

    }
    return 0;
}
void Scene::startLoadingThread()
{
    //_window.startThread(modelLoadingThread,this);
    _threadHandle=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE) modelLoadingThread,this,0,&_threadID);
}
void Scene::closeLoadingThread()
{
    _stopThread=true;

    WaitForMultipleObjects(1, &_threadHandle, TRUE, INFINITE);
    CloseHandle(_threadHandle);
}
int modelLoadingThread(void* world)
{
    Scene* w=(Scene*)world;
    return w->loadingThread();
}
std::string Scene::getFileName(const char* extension)
{
    char dir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH,dir);

    char name[MAX_PATH];
    OPENFILENAME ofile;
    ZeroMemory(&ofile,sizeof(ofile));
    ofile.lStructSize= sizeof(ofile);
    ofile.nMaxFile=MAX_PATH;
    ofile.lpstrFilter=extension;
    ofile.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofile.lpstrFile=name;
    ofile.lpstrFile[0]='\0';
    GetOpenFileName(&ofile);

    SetCurrentDirectory(dir);
    return std::string(ofile.lpstrFile);
}
Model* Scene::getTargetModel()
{
    float intMax=99;
    float intersectionDistance;
    glm::vec3 ray=glm::normalize( glm::vec3(_models[0]->rotationMatrix*glm::vec4(0.0f,0.0f,-1.0f,1.0f) ) );
    _targetModel=0;

    for(unsigned int a=_3D_MODELS_BEGIN;a<_models.size(); a++)//skips the HDRI
    {
        if (_models[a]->shown && ge::intersetRayModel(_models[a], _models[0]->position, ray,intersectionDistance))
        {
            if (intersectionDistance<intMax)
            {
                intMax=intersectionDistance;
                _targetModel=_models[a];
            }
        }
    }
    return _targetModel;
}

