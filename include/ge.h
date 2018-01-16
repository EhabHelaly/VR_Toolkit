#ifndef GAME_ENGINE_H
#define GAME_ENGINE_H

#include <glm/glm.hpp>

/// Game Engine Common Enumerations and Macros

/// Transformation Perspectives
//   specifies reference XYZ axes used in translation / rotation / scaling
//   some perspectives are are irrelevant in some cases
enum TransformationPerspectives{
    GE_PERSPECTIVE_WORLD,
    GE_PERSPECTIVE_OBJECT,
    GE_PERSPECTIVE_CAMERA,
    GE_PERSPECTIVE_FULL_CAMERA,
};

/// Light Sources
#define LIGHT_SOURCES_MAX 5 // must change the same macro in shader
enum LightSourcesTypes{
    LIGHT_SOURCE_OFF,
    LIGHT_SOURCE_DIRECTIONAL,
    LIGHT_SOURCE_POINT,
    LIGHT_SOURCE_POINT_REALISTIC,
    LIGHT_SOURCE_SPOT,// not yet supported in shader
};
typedef struct
{
    int       type      = LIGHT_SOURCE_OFF;
    float     power     = 100;
    glm::vec3 position  = glm::vec3(0,10,0);
    glm::vec3 color     = glm::vec3(1,1,1);
    glm::vec3 direction = glm::vec3(0,1,1);
}Light;


#endif // GAME_ENGINE_H
