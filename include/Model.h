#ifndef VR_MODEL_H
#define VR_MODEL_H

#include "Map.h"
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

enum AttributeIndex{
    // vbo needed
    ATR_INDEX_POSITION, // vertex positions
    ATR_INDEX_COLOR,    // vertex texture
    ATR_INDEX_NORM,     // vertex normal (face normal)
    ATR_INDEX_TANG,     // vertex tangent
    ATR_INDEX_BITANG,   // vertex bitangent
};

enum FileParsingResult{
    PARSING_PENDING,
    PARSING_FAILED,
    PARSING_SUCCEEDED,
};
/**
* Class Material *
*
* stores image IDs (texture/normal/glossy/...) and default color of an object
* same material can be used for multiple objects in the same model
*/

typedef struct {
    std::string name;
    GLuint mAmbient =0;
    GLuint mDiffuse =0;
    GLuint mNormal  =0;
    GLuint mSpecular=0;
    GLuint mEmission=0;
    GLuint mDepth   =0;
    glm::vec3 ambient =glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 diffuse =glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 specular=glm::vec3(0.5f,0.5f,0.5f);
    glm::vec3 emission=glm::vec3(0.0f,0.0f,0.0f);
    float     depth   =1.0f;
}Material;


class Object
{
public:
	Object();
	void release();				// clean up object

	void initBuffers();
	void draw();
	std::vector<glm::vec3>     vertexBuffer;
	std::vector<glm::vec2>    textureBuffer;
	std::vector<glm::vec3>    normalsBuffer;
	std::vector<glm::vec3>   tangentsBuffer;
	std::vector<glm::vec3> bitangentsBuffer;

	std::string name="";

	glm::vec3 position=glm::vec3(0,0,0);
	glm::vec3 rotation=glm::vec3(0,0,0);
	glm::vec3 scaling =glm::vec3(1,1,1);

	glm::mat4 translationMatrix=glm::mat4(1.0f);
	glm::mat4    rotationMatrix=glm::mat4(1.0f);
	glm::mat4     scalingMatrix=glm::mat4(1.0f);
	glm::mat4       modelMatrix=glm::mat4(1.0f);
	Material* material;
	std::string materialName="";

	int numVertices;
    GLuint vbo[5];
    GLuint vao[1];
private:
    void _generateNormals();
    void _generateTangentsAndBtangents();

};

class Model
{
public:
	Model();
	~Model();
	void release();// clean up model
	std::string modelFilePath;
	std::string modelFileName;
	std::string materialFileName;
	std::vector<Object> objects;
    std::vector<Material*> materials;

    bool shown=false;
    int parsed=PARSING_PENDING;

	// Model Mesh Specs
	float radius=0;
	glm::vec3 center;
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

	// Model transformation Specs
	glm::vec3 position   =glm::vec3(0.0f,0.0f,0.0f);;
	glm::vec3 orientation=glm::vec3(0.0f,0.0f,0.0f);;
	glm::vec3 scaling    =glm::vec3(1.0f,1.0f,1.0f);

	glm::mat4 translationMatrix=glm::mat4(1.0f);
	glm::mat4    rotationMatrix=glm::mat4(1.0f);
	glm::mat4     scalingMatrix=glm::mat4(1.0f);
	glm::mat4       modelMatrix=glm::mat4(1.0f);

	// Model methods
	bool init();
	bool load();
	bool load(std::string fileName, std::string mFileName="");
    bool updateMaterials(std::string materialFileName);
	Material* getMaterial(std::string materialName);

	Object* newObject();
	void delObject(int id); // releases the object then removes it from objects vector
private:
    bool loadModel_obj();
    bool loadMaterials_mtl(std::string materialFileName);

};

#endif
