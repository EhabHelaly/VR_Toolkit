

#include "Errors.h"
#include <SDL2/SDL.h>
#include <cstdlib>

void fatalError(std::string errorString){
    SDL_Quit(); //to quit the SDL
    std::cout << errorString << std::endl;
    system("pause");
    exit(69); //any thing unless 0

}

