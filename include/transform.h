#ifndef TRANSFORM_H_INC
#define TRANSFORM_H_INC

#include "glm/glm.hpp"
#include "Model.h"
#include "Camera.h"
#include "ge.h"

namespace ge
{
void translate       (Model* model, Camera* camera, glm::vec3 translation, int perspective=GE_PERSPECTIVE_CAMERA);
void translateTo     (Model* model, Camera* camera, glm::vec3 translation, int perspective=GE_PERSPECTIVE_CAMERA);

void rotate          (Model* model, Camera* camera, glm::vec3 angle      , int perspective=GE_PERSPECTIVE_WORLD);
void rotateTo        (Model* model, Camera* camera, glm::vec3 angle      , int perspective=GE_PERSPECTIVE_WORLD);

void scale           (Model* model, Camera* camera, glm::vec3 scaleFactor, int perspective=GE_PERSPECTIVE_OBJECT);
void scaleTo         (Model* model, Camera* camera, glm::vec3 scaleFactor, int perspective=GE_PERSPECTIVE_OBJECT);

bool intersetRayModel(Model* model,glm::vec3 origin, glm::vec3 direction, float &intersectionDistance);
}


#endif // TRANSFORM_H_INC
