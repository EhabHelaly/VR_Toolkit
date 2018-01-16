#ifndef GLSLPROGRAM_H
#define GLSLPROGRAM_H
#include <iostream>
#include <string.h>
#include <GL/glew.h>
#include <glm/glm.hpp>


//a program that runs our GLSL code
//GLSL -> openGL shading language
//two things we will d0: 1) compile the program (take all the text and compile it into a form that our GPU can use)
//2) link the program (take our vertex shader and fragment shader and link them into one shader program that GPU can use )
class GLSLProgram
{
    public:
        GLSLProgram();
        ~GLSLProgram();

        //function to compile the shaders //strings -> file path
        //Read the file them compile it into the format that openGL can use
        void compileShaders(const std::string vertexShaderFilePath, const std::string fragmentShaderFilePath);
        void linkShaders(); //link the shaders together into the final program
        void addAttribute(const std::string attributeName); //between compiling and linking you need to bind our attributes and the inputs in vertex shader
        //so bind the vertex attributes to the string // then our input will be the first attribute

        GLint getUniformLocation(const std::string uniform);

        void setMat4 (const std::string uniform, const glm::mat4 value);
        void setMat3 (const std::string uniform, const glm::mat3 value);
        void setVec3 (const std::string uniform, const glm::vec3 value);
        void setInt  (const std::string uniform, const int       value);
        void setFloat(const std::string uniform, const float     value);

        void use();
        void unuse();

        GLuint _progID;

    private:

        void compileShader(const std::string filePath, GLuint id);

        //you need id to hold onto your shader program (the handle for our entire openGL program)
        //every object in openGL needs a GLuint -> vbo, textures programs
        GLuint _vertexShaderID;
        GLuint _fragmentShaderID;
        int _numAttributes; //counts the number of attributes that we have //may position, color, textures
};

#endif // GLSLPROGRAM_H
