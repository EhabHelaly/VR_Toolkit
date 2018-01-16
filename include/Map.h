#ifndef MAP_H
#define MAP_H

#include <string>
#include <GL/glew.h>


enum MapType{
    MAP_AMBIENT,
    MAP_DIFFUSE,
    MAP_NORMAL,
    MAP_SPECULAR,
    MAP_EMISSION,
    MAP_DEPTH,
};

GLuint loadMap(std::string path);
GLuint getMapID(std::string path);
void deleteMap(GLuint mapID);
void deleteMaps();

#endif // MAP_H
