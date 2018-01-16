#include "Camera.h"
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

Camera::Camera()
{}

void Camera::init(int screenWidth, int screenHeight)
{
    width  = screenWidth;
    height = screenHeight;
    fovy   = 45.0f;

    _projectMatrix = glm::perspective
    (
        glm::radians(fovy),          //Field of view
        width / (float)height,       //Aspect ratio
        0.01f,                       //Near clipping plane
        500.0f                       //Far clipping plane
    );

    viewMatrix = glm::lookAt
		(
			glm::vec3(0,0,0), // Camera is  at the origin
			glm::vec3(0,0,-1),// And looks at (?,?,?), in World Space
			glm::vec3(0,1,0)  // Head is up ( set to 0,-1,0 to look upside-down )
		);

    _originalViewMatrix = viewMatrix;
}
void Camera::translateTo(glm::vec3 translation, int perspective)
{
    position = glm::vec3(0,0,0);
    translate(translation,perspective);
}
void Camera::translate(glm::vec3 translation, int perspective)
{
    glm::vec3 X = glm::vec3(1, 0, 0);
    glm::vec3 Y = glm::vec3(0, 1, 0);
    glm::vec3 Z = glm::vec3(0, 0, 1);

    switch(perspective)
    {
        case GE_PERSPECTIVE_FULL_CAMERA:
        {
            X=glm::vec3(glm::inverse(rotationMatrix)*glm::vec4(X,1));
            Y=glm::vec3(glm::inverse(rotationMatrix)*glm::vec4(Y,1));
            Z=glm::vec3(glm::inverse(rotationMatrix)*glm::vec4(Z,1));
            break;
        }
        case GE_PERSPECTIVE_CAMERA:
        {
            // consider only the yaw angle
            X=glm::rotate(X, -1.0f*glm::radians(orientation.y), Y);
            Z=glm::rotate(Z, -1.0f*glm::radians(orientation.y), Y);
            break;
        }
        case GE_PERSPECTIVE_WORLD:
        {
            // no need to change axes
            break;
        }
    }

    position += translation.x* X+ translation.y* Y+ translation.z* Z;
    // XYZ-axis
    translationMatrix  = glm::translate(glm::mat4(1.0f), -1.0f*position) ;

    updateViewMatrix();
}
void Camera::rotateTo(glm::vec3 angle)
{
    orientation=glm::vec3(0,0,0);
    rotate(angle);
}
void Camera::rotate(glm::vec3 angle)
{
    orientation+= angle;
    angle=glm::radians(orientation);

    rotationMatrix=glm::eulerAngleZ(angle.z) * glm::eulerAngleX(angle.x) * glm::eulerAngleY(angle.y);
    updateViewMatrix();
}
void Camera::rotateYPR(glm::vec3 angle)
{
    orientation=angle;
    angle=glm::radians(orientation);

    glm::mat4 rotation=glm::eulerAngleZ(angle.z) * glm::eulerAngleX(angle.x) * glm::eulerAngleY(angle.y);

    glm::mat4 orignal = glm::lookAt
		(
			glm::vec3(0,0,0), // Camera is at (?,?,?), in World Space
			glm::vec3(0,-1,0),  // And looks at the origin
			glm::vec3(0,0,1)  // Head is up ( set to 0,-1,0 to look upside-down )
		);

    viewMatrix = rotation * translationMatrix * orignal;
}

void Camera::updateViewMatrix()
{
    viewMatrix = rotationMatrix * translationMatrix * _originalViewMatrix;
}
