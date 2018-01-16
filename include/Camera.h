#ifndef CAMERA_H
#define CAMERA_H
#include "glm/glm.hpp"
#include "ge.h"
class Camera
{
    public:
        Camera();
        void init(int screenWidth, int screenHeight);
        void translate  (glm::vec3 translation, int perspective=GE_PERSPECTIVE_CAMERA);
        void translateTo(glm::vec3 translation, int perspective=GE_PERSPECTIVE_CAMERA);
        void rotate     (glm::vec3 angle);
        void rotateTo   (glm::vec3 angle);
        void rotateYPR  (glm::vec3 angle);

        int width;
        int height;
        float fovy;
        glm::mat4 getCameraMatrix()      { return _projectMatrix * viewMatrix; }
        glm::mat4 getProjectionMatrix()  { return _projectMatrix; }

        glm::vec3 position    = glm::vec3(0,0,0);
        glm::vec3 orientation = glm::vec3(0,0,0);

        glm::mat4 viewMatrix;
        glm::mat4 rotationMatrix;
        glm::mat4 translationMatrix;

    private:
        glm::mat4 _projectMatrix;
        glm::mat4 _originalViewMatrix;
        void updateViewMatrix();


};

#endif // CAMERA_H
