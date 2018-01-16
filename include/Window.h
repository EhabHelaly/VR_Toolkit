#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <string>

enum WindowFlags {INVISIBLE = 0x1, FULLSCREEN = 0x2, BORDERLESS = 0x4};

enum WindowParams{
    GE_WINDOW_WIDTH,
    GE_WINDOW_HEIGHT,
    GE_WINDOW_FPS,
    GE_WINDOW_TIME,
    GE_WINDOW_TIME_DELTA,
};
class Window
{
public:
    Window();
    int create(std::string windowName, int screenWidth, int screenHeight, unsigned int currentFalgs);
    void release();
    void startThread(int (*threadFunction)(void*), void* data);
    void bindContext();
    void unbindContext();
    void swapBuffer();
    void  set(int windowParam, float value);
    float get(int windowParam);
    float calculateFPS(); //returns the current FPS

private:

    SDL_Window* _sdlWindow;
    SDL_GLContext _glContextMain;
    SDL_GLContext _glContextThread;
    SDL_Thread *_thread;

    int _screenWidth;
    int _screenHeight;
    float _fps=30;
    int _timePerFrame=1000/30;
    int _timeLastFrame;
    void syncFPS();

    float _fpsCalculated;
    float _frameTime;
    float _frameTimeOld;

};

#endif // WINDOW_H
