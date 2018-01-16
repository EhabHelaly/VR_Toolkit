#include "transform.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/intersect.hpp>
#include <glm/gtx/euler_angles.hpp>

static void updateModelMatrix(Model* model);

// Translate the model
void ge::translateTo(Model* model, Camera* camera, glm::vec3 translation, int perspective)
{
    model->position=glm::vec3(0.0f,0.0f,0.0f);
    ge::translate( model, camera, translation, perspective);
}
void ge::translate(Model* model, Camera* camera, glm::vec3 translation, int perspective)
{
    glm::vec3 X = glm::vec3(1, 0, 0);
    glm::vec3 Y = glm::vec3(0, 1, 0);
    glm::vec3 Z = glm::vec3(0, 0, 1);

    switch(perspective)
    {
        case GE_PERSPECTIVE_FULL_CAMERA:
        {
            X=glm::vec3(glm::inverse(camera->rotationMatrix)*glm::vec4(X,1));
            Y=glm::vec3(glm::inverse(camera->rotationMatrix)*glm::vec4(Y,1));
            Z=glm::vec3(glm::inverse(camera->rotationMatrix)*glm::vec4(Z,1));
            break;
        }
        case GE_PERSPECTIVE_CAMERA:
        {
            X=glm::rotate(X, -1*glm::radians(camera->orientation.y), Y);
            Z=glm::rotate(Z, -1*glm::radians(camera->orientation.y), Y);
            break;
        }
        case GE_PERSPECTIVE_WORLD:
        {
            // no need to change axes
            break;
        }
        case GE_PERSPECTIVE_OBJECT:
        {
            // translate object based on its own orientation
            X=glm::vec3(glm::inverse(model->rotationMatrix)*glm::vec4(X,1));
            Y=glm::vec3(glm::inverse(model->rotationMatrix)*glm::vec4(Y,1));
            Z=glm::vec3(glm::inverse(model->rotationMatrix)*glm::vec4(Z,1));
            break;
        }
    }

    model->position += translation.x* X+ translation.y* Y+ translation.z* Z;
    model->translationMatrix  = glm::translate(glm::mat4(1.0f), model->position) ;

    updateModelMatrix(model);
}

// Rotate the model
void ge::rotateTo(Model* model, Camera* camera, glm::vec3 angle, int perspective)
{
    model->orientation=glm::vec3(0.0f,0.0f,0.0f);
    ge::rotate( model, camera, angle, perspective);
}
void ge::rotate(Model* model, Camera* camera, glm::vec3 angle, int perspective)
{
    // Current perspective: world
    // other perspectives are irrelevant, yet they can be implemented later
    model->orientation += angle;
    angle=glm::radians(model->orientation);
    model->rotationMatrix= glm::eulerAngleY(angle.y) * glm::eulerAngleX(angle.x) * glm::eulerAngleZ(angle.z) ;

    updateModelMatrix(model);
}

//Scale the model
void ge::scaleTo(Model* model, Camera* camera, glm::vec3 scaleFactor, int perspective)
{
    // Current perspective: object
    // other perspectives are irrelevant, yet they can be implemented later
    model->scaling=glm::vec3(1.0f,1.0f,1.0f);
    model->scalingMatrix=glm::mat4(1.0f);
    ge::scale( model, camera, scaleFactor, perspective);
}
void ge::scale(Model* model, Camera* camera, glm::vec3 scaleFactor, int perspective)
{
    model->scaling *= scaleFactor;

    model->scalingMatrix = glm::scale(model->scalingMatrix, scaleFactor);
    updateModelMatrix(model);
}

bool ge::intersetRayModel(Model* model,glm::vec3 origin, glm::vec3 direction,float &intersectionDistance)
{
    glm::vec3 intersection;
    int num_intersections=0;
    intersectionDistance=99;

    glm::vec3 t1 = model->modelMatrix * glm::vec4(model->minPoint.x, model->minPoint.y, model->minPoint.z, 1);
    glm::vec3 t2 = model->modelMatrix * glm::vec4(model->maxPoint.x, model->minPoint.y, model->minPoint.z, 1);
    glm::vec3 t3 = model->modelMatrix * glm::vec4(model->maxPoint.x, model->minPoint.y, model->maxPoint.z, 1);
    glm::vec3 t4 = model->modelMatrix * glm::vec4(model->minPoint.x, model->minPoint.y, model->maxPoint.z, 1);

    glm::vec3 t5 = model->modelMatrix * glm::vec4(model->minPoint.x, model->maxPoint.y, model->minPoint.z, 1);
    glm::vec3 t6 = model->modelMatrix * glm::vec4(model->maxPoint.x, model->maxPoint.y, model->minPoint.z, 1);
    glm::vec3 t7 = model->modelMatrix * glm::vec4(model->maxPoint.x, model->maxPoint.y, model->maxPoint.z, 1);
    glm::vec3 t8 = model->modelMatrix * glm::vec4(model->minPoint.x, model->maxPoint.y, model->maxPoint.z, 1);

    // 6 faces 12 triangles
    // 1234 5678 1265 3487 1485 2376 --> Faces
    // 123  567  126  348  148  237  --> triangles (1st half)
    //  134  578  165  387  185  276 --> triangles (2st half)
    glm::vec3 tri[12][3]={
        // Face 1 (1234)
        {t1,t2,t3},
        {t1,t3,t4},
        // Face 2 (5678)
        {t5,t6,t7},
        {t5,t7,t8},
        // Face 3 (1265)
        {t1,t2,t6},
        {t1,t6,t5},
        // Face 4 (3487)
        {t3,t4,t8},
        {t3,t8,t7},
        // Face 5 (1485)
        {t1,t4,t8},
        {t1,t8,t5},
        // Face 6 (2376)
        {t2,t3,t7},
        {t2,t7,t6}
    };

    for(int t=0;t<12;t++)
    {
        if (glm::intersectRayTriangle(origin, direction, tri[t][0], tri[t][1], tri[t][2],intersection) )
        {
            num_intersections++;
            float length=glm::length(intersection-origin);
            intersectionDistance= length<intersectionDistance? length:intersectionDistance;
        }
        if (num_intersections>=2)
        {
            break;
        }
    }

    return num_intersections>=2;
}


static void updateModelMatrix(Model* model)
{
    model->modelMatrix = model->translationMatrix * model->rotationMatrix * model->scalingMatrix ;
}
