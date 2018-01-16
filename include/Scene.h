// standard
#include <vector>
#include <string>
#include <windows.h>
// VR
#include "VrKit.h"
#include "VrApp.h"
// Game Engine
#include "GLSLProgram.h"
#include "transform.h"
#include "Window.h"
#include "Camera.h"
#include "Model.h"
#include "ge.h"

#define _3D_MODELS_BEGIN 3 // 0,1,2 are ray/hdri/plane

enum class AppState {PLAY, EXIT};

class Scene
{
public:

	Scene(); //initialize SDL, Shader, Camera
	void release();
	void run();
	void processInputSDL();
	bool isActive();

	// selection related
    Model* getTargetModel();
    glm::vec3 getTargetPosXZ();
    // Model related
	Model* newModel();	 // adds a new model to the models vector, and loads it's data unless input string is empty
    Model* newModel2D();
	void delModel(Model* model);  	// releases model then removes it from models vector
	bool loadScene(std::string ivrFileName);
	bool saveScene(std::string ivrFileName);
    std::string getFileName(const char* extension="All\0*.*\0");
    bool intersetRayModel(glm::vec3 origin, glm::vec3 direction,Model* model,float &intersectionDistance);
    // rendering related
	void render();    		// clears GL buffers, uses shader, renders all models, and swap buffer
	void renderModel(Model* model, bool transparency);
    void renderModel2D(Model* model);
    // thread related
    int loadingThread();
    void startLoadingThread();
    void closeLoadingThread();

    // VR app functions
    void initialzeVR();
    void processInputVR();

private:

    // Game Engine
    Window _window;
	Camera _camera;                     // scene's camera
	Light _lights[LIGHT_SOURCES_MAX];
	GLSLProgram _shader;	            // scene's shader program
	std::vector<Model*> _models;        // scene's loaded models
	std::vector<Model*> _models_2D;     // scene's loaded models (2D)

	// other
	Model* _activeModel=nullptr;             // pointer to the current active model to be edited
	Model* _targetModel=nullptr;             // pointer to the current active model to be edited
	bool   _loadingStatus=false;
	AppState _appState=AppState::PLAY;
    float _fps;

    // thread related
    bool      _stopThread=false;
    bool _threadIsRunning=false;
    HANDLE  _threadHandle;
    DWORD   _threadID;

    // VR App related
    VrKit _vrKit;
    VrApp _vrApp;

};
