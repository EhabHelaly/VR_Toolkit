#include "Window.h"
#include "Errors.h"

Window::Window() : _sdlWindow(nullptr)
{
}

int Window::create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFalgs) {

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_SHARE_WITH_CURRENT_CONTEXT, 1);

    Uint32 flags = SDL_WINDOW_OPENGL;

    if(currentFalgs & INVISIBLE) {
        flags |= SDL_WINDOW_HIDDEN;
    }
    if(currentFalgs & FULLSCREEN) {
        flags |= SDL_WINDOW_FULLSCREEN;
    }
    if(currentFalgs & BORDERLESS) {
        flags |= SDL_WINDOW_BORDERLESS;
    }


    _screenHeight = screenHeight;
    _screenWidth = screenWidth;

     _sdlWindow = SDL_CreateWindow(windowName.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, _screenWidth, _screenHeight, flags);
    if(_sdlWindow == nullptr){
        fatalError("SDL window could not be created! ");
    }
    _glContextThread= SDL_GL_CreateContext(_sdlWindow);
    if(_glContextThread == nullptr) {
        fatalError("SDL_GL context could not be created!");
    }
    _glContextMain= SDL_GL_CreateContext(_sdlWindow);
    if(_glContextMain == nullptr) {
        fatalError("SDL_GL context could not be created!");
    }
    SDL_GL_MakeCurrent(_sdlWindow, _glContextMain);

    GLenum error = glewInit();
    if(error != GLEW_OK){
        fatalError("Could not initialize glew!");
    }

    glEnable(GL_DEPTH_TEST); //Do depth comparisons and update the depth buffer
	glDepthFunc(GL_LEQUAL);    // Accept fragment if it closer to the camera than the former one
    glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glShadeModel(GL_SMOOTH);

    printf("**** OpenGL Version: %s ****\n", glGetString(GL_VERSION));

    glClearColor(0.8f, 0.8f, 1.0f, 1.0f); //Setup the background color

    //set VSYNC
    SDL_GL_SetSwapInterval(1);
    _timeLastFrame=SDL_GetTicks();

    return 0;

}
void Window::startThread(int (*threadFunction)(void*), void* data)
{
    _thread = SDL_CreateThread((SDL_ThreadFunction)(*threadFunction), "Thread", data);
}
void Window::bindContext()
{
    SDL_GL_MakeCurrent(_sdlWindow, _glContextThread);
}
void Window::unbindContext()
{
    SDL_GL_MakeCurrent(_sdlWindow, NULL);
}
void Window::release()
{
    //int threadReturnValue;
    //SDL_WaitThread(_thread, &threadReturnValue);
    SDL_GL_DeleteContext(_glContextThread);
    SDL_GL_DeleteContext(_glContextMain);
    SDL_DestroyWindow(_sdlWindow);
    SDL_Quit();
}
void Window::swapBuffer() {
    SDL_GL_SwapWindow(_sdlWindow);
    syncFPS();
}
void Window::set(int windowParam, float value)
{
    switch (windowParam)
    {
        case GE_WINDOW_HEIGHT: {_screenHeight=value; break;}
        case GE_WINDOW_WIDTH : {_screenWidth =value; break;}
        case GE_WINDOW_FPS   : {_fps         =value;_timePerFrame=1000/_fps; break;}
        default              : {                     break;}
    }
}
float Window::get(int windowParam)
{
    float value=0;
    switch (windowParam)
    {
        case GE_WINDOW_HEIGHT: {value=_screenHeight ; break;}
        case GE_WINDOW_WIDTH : {value=_screenWidth  ; break;}
        case GE_WINDOW_FPS   : {value=_fps          ; break;}
        case GE_WINDOW_TIME  : {value=SDL_GetTicks(); break;}
        case GE_WINDOW_TIME_DELTA  : {value=SDL_GetTicks()-_frameTimeOld;_frameTimeOld=SDL_GetTicks(); break;}
        default              : {                      break;}
    }
    return value;
}
void Window::syncFPS()
{
    int time=SDL_GetTicks();
    int timeDelay=_timePerFrame - (time-_timeLastFrame);
    if (timeDelay>0)
    {
        SDL_Delay(timeDelay);
    }
    _timeLastFrame=time;
}
float Window::calculateFPS() {

    static const int NUM_SAMPLES = 10;    //number of frames we will average
    static float frameTimes[NUM_SAMPLES];
    static int currentFrame = 0;


    static float prevTicks = SDL_GetTicks();

    float currentTicks;
    currentTicks = SDL_GetTicks();

    _frameTime = currentTicks - prevTicks; //Note: for the first frame only the prev = current ! wrong !

    prevTicks = currentTicks;

    frameTimes[currentFrame % NUM_SAMPLES] = _frameTime;
    currentFrame++;

    int count = 0;
    if(currentFrame < NUM_SAMPLES) {
        count = currentFrame;
    } else {
        count = NUM_SAMPLES;
    }

    float frameTimeAverage = 0;
    for(int i = 0; i < count; i++) {
            frameTimeAverage += frameTimes[i];
    }

    frameTimeAverage /= count;
    _fpsCalculated = 1000.0f / frameTimeAverage;

    return _fpsCalculated;
}

