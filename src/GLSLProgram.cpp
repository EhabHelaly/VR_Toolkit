#include "GLSLProgram.h"
#include <iostream>
#include "Errors.h"
#include <fstream>
#include <vector>




GLSLProgram::GLSLProgram():
    _progID(0), _vertexShaderID(0), _fragmentShaderID(0), _numAttributes(0)

{}

GLSLProgram::~GLSLProgram()
{}

void GLSLProgram::compileShaders(const std::string vertexShaderFilePath, const std::string fragmentShaderFilePath){

    _progID = glCreateProgram();

    _vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    if(_vertexShaderID == 0){
        fatalError("Vertex shader failed to be created");
    }

    _fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    if(_fragmentShaderID == 0){
        fatalError("Fragment shader failed to be created");
    }

    compileShader(vertexShaderFilePath, _vertexShaderID);
    compileShader(fragmentShaderFilePath, _fragmentShaderID);


}

void GLSLProgram::linkShaders(){

    glAttachShader(_progID, _vertexShaderID);
    glAttachShader(_progID, _fragmentShaderID);

    glLinkProgram(_progID);

    GLint isLinked = 0;
    glGetProgramiv(_progID, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(_progID, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<char> errorLog(maxLength);
        glGetProgramInfoLog(_progID, maxLength, &maxLength, &errorLog[0]);

        glDeleteProgram(_progID);

        glDeleteShader(_vertexShaderID);
        glDeleteShader(_fragmentShaderID);

        printf("%s/n", &errorLog[0]);
        fatalError("Shaders failed to link!");

}

    glDetachShader(_progID, _vertexShaderID);
    glDetachShader(_progID, _fragmentShaderID);
    glDeleteShader(_vertexShaderID);
    glDeleteShader(_fragmentShaderID);

}

void GLSLProgram::addAttribute(const std::string attributeName){
    glBindAttribLocation(_progID, _numAttributes++, attributeName.c_str());

}

GLint GLSLProgram::getUniformLocation(const std::string uniform){
    GLuint location = glGetUniformLocation(_progID, uniform.c_str());
    if(location == GL_INVALID_INDEX){
        fatalError("Uniform " + uniform + " not found in shader!");
    }
    return location;
}
void GLSLProgram::setMat4(const std::string uniform, const glm::mat4 value)
{
    glUniformMatrix4fv(getUniformLocation(uniform), 1, GL_FALSE, &value[0][0]);
}
void GLSLProgram::setMat3(const std::string uniform, const glm::mat3 value)
{
    glUniformMatrix3fv(getUniformLocation(uniform), 1, GL_FALSE, &value[0][0]);
}
void GLSLProgram::setVec3(const std::string uniform, const glm::vec3 value)
{
    glUniform3f(getUniformLocation(uniform),value.x,value.y,value.z);
}
void GLSLProgram::setInt(const std::string uniform, const int value)
{
    glUniform1i(getUniformLocation(uniform), value);
}
void GLSLProgram::setFloat(const std::string uniform, const float value)
{
    glUniform1f(getUniformLocation(uniform),value);
}
// Real time
void GLSLProgram::use(){
    glUseProgram(_progID);

    for(int i = 0; i < _numAttributes; i++){
        glEnableVertexAttribArray(i);
    }
}

void GLSLProgram::unuse(){
    glUseProgram(0);
    for(int i = 0; i < _numAttributes; i++){
        glDisableVertexAttribArray(i);
    }
}

void GLSLProgram::compileShader(const std::string filePath, GLuint id){

    std::ifstream File(filePath);
    if(File.fail()){
        perror(filePath.c_str());
        fatalError("Failed to open " + filePath);
    }
    std::string fileContents = "";
    std::string line;
    while(std::getline(File, line)){
            fileContents += line + "\n";
    }
    File.close();

    const char* contentsptr = fileContents.c_str();
    glShaderSource(id, 1, &contentsptr, nullptr);
    glCompileShader(id);

    //Error checking
    GLint success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if(success == GL_FALSE){
	GLint maxLength = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);


	std::vector<GLchar> errorLog(maxLength);
	glGetShaderInfoLog(id, maxLength, &maxLength, &errorLog[0]);

	glDeleteShader(id);

	printf("%s\n", &errorLog[0]);

	fatalError("shader" + filePath + "failed to compile");
	}
}
