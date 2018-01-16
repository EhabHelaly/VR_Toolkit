#include "Model.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <opencv2/opencv.hpp>
using namespace std;

typedef struct {
	string materialName;
	int numVertices;
    GLuint vbo[5];
    GLuint vao[1];
}SharedObject;

typedef struct {
	string modelFileName;
	string materialFileName;
	vector<SharedObject*> sh_objects;
	// Model Mesh Specs
	float radius=0;
	glm::vec3 center;
	glm::vec3 minPoint;
	glm::vec3 maxPoint;

	int used=0;
}SharedModel;

static vector<SharedModel*> sh_models;
static SharedModel* isModelShared(std::string path);
static int sharedModelIndex(SharedModel* sh_model);

// Object
Object::Object()
{

}
void Object::release()
{
	// perform any clean up operations needed before erasing the object from the  objects vector
	glDeleteBuffers(5,vbo);
	glDeleteVertexArrays(1,vao);
}
void Object::initBuffers()
{
    numVertices = vertexBuffer.size();

    // Generate and assign two Vertex Buffer Objects to our handle
    glGenBuffers(5, vbo);
    // Generate and assign a Vertex Array Object to our handle
    glGenVertexArrays(1, vao);
    // Bind our Vertex Array Object as the current used object
    glBindVertexArray(vao[0]);


    // Positions
    // ===================
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    // Copy the vertex data from diamond to our buffer
    glBufferData(GL_ARRAY_BUFFER, vertexBuffer.size() * sizeof(glm::vec3), &vertexBuffer[0], GL_STATIC_DRAW);
    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
    glVertexAttribPointer(ATR_INDEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0 );

    // Texture coordinates
    // ===============
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    // Copy the vertex data from diamond to our buffer
    glBufferData(GL_ARRAY_BUFFER,  textureBuffer.size() * sizeof(glm::vec2), &textureBuffer[0], GL_STATIC_DRAW);
    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
    glVertexAttribPointer(ATR_INDEX_COLOR, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Normals
    // =======================
    if (normalsBuffer.size()==0)
    {
        _generateNormals();
    }
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    // Copy the vertex data from diamond to our buffer
    glBufferData(GL_ARRAY_BUFFER,  normalsBuffer.size() * sizeof(glm::vec3), &normalsBuffer[0], GL_STATIC_DRAW);
    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
    glVertexAttribPointer(ATR_INDEX_NORM, 3, GL_FLOAT, GL_FALSE, 0, 0);


    // Tangents
    // =======================
    _generateTangentsAndBtangents();
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    // Copy the vertex data from diamond to our buffer
    glBufferData(GL_ARRAY_BUFFER,  tangentsBuffer.size() * sizeof(glm::vec3), &tangentsBuffer[0], GL_STATIC_DRAW);
    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
    glVertexAttribPointer(ATR_INDEX_TANG, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Bitangents
    // =======================
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    // Copy the vertex data from diamond to our buffer
    glBufferData(GL_ARRAY_BUFFER,  bitangentsBuffer.size() * sizeof(glm::vec3), &bitangentsBuffer[0], GL_STATIC_DRAW);
    // Specify that our coordinate data is going into attribute index 0, and contains three floats per vertex
    glVertexAttribPointer(ATR_INDEX_BITANG, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Enable our attributes within the current VAO
    glEnableVertexAttribArray(ATR_INDEX_POSITION);
    glEnableVertexAttribArray(ATR_INDEX_COLOR);
    glEnableVertexAttribArray(ATR_INDEX_NORM);
    glEnableVertexAttribArray(ATR_INDEX_TANG);
    glEnableVertexAttribArray(ATR_INDEX_BITANG);

    // Set up shader
    // ===================
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(vao[0]);

    // Enable our attributes within the current VAO
    glDisableVertexAttribArray(ATR_INDEX_POSITION);
    glDisableVertexAttribArray(ATR_INDEX_COLOR);
    glDisableVertexAttribArray(ATR_INDEX_NORM);
    glDisableVertexAttribArray(ATR_INDEX_TANG);
    glDisableVertexAttribArray(ATR_INDEX_BITANG);
    // clear all buffers
    vertexBuffer.clear();
    textureBuffer.clear();
    normalsBuffer.clear();
    tangentsBuffer.clear();
    bitangentsBuffer.clear();
}
void Object::draw()
{
    /// VAO HAS BUGS WHEN LOADING MODELS IN REALTIME (EVEN IF LOADED IN SAME RENDERING THREAD)
    /// BINDING VBO INSTEAD OF VAO GETS THE JOB DONE UNTIL WE KNOW THE CAUSE OF THE PROBLEM
    //glBindVertexArray(vao[0]);/*
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glVertexAttribPointer(ATR_INDEX_POSITION, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glVertexAttribPointer(ATR_INDEX_COLOR   , 2, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[2]);
    glVertexAttribPointer(ATR_INDEX_NORM    , 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[3]);
    glVertexAttribPointer(ATR_INDEX_TANG    , 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[4]);
    glVertexAttribPointer(ATR_INDEX_BITANG  , 3, GL_FLOAT, GL_FALSE, 0, 0);//*/

    glDrawArrays(GL_TRIANGLES , 0, numVertices);

}
void Object::_generateNormals()
{
    glm::vec3 v1,v2,vn;
    normalsBuffer.resize(vertexBuffer.size());
    for(unsigned int point=0; point<vertexBuffer.size();point+=3)
    {
        v1=vertexBuffer[point]-vertexBuffer[point+1];
        v2=vertexBuffer[point]-vertexBuffer[point+2];
        vn=glm::cross(v1,v2);
        normalsBuffer[point  ]=vn;
        normalsBuffer[point+1]=vn;
        normalsBuffer[point+2]=vn;
    }
}
void Object::_generateTangentsAndBtangents()
{
    glm::vec3 v1,v2;
    glm::vec2 u1,u2;
    glm::vec3 tangent;
    glm::vec3 bitangent;

    tangentsBuffer.resize(vertexBuffer.size());
    bitangentsBuffer.resize(vertexBuffer.size());
    for (unsigned int point=0; point<vertexBuffer.size(); point+=3)
    {
        // Edges of the triangle : position delta
        v1 = vertexBuffer[point+1]-vertexBuffer[point+0];
        v2 = vertexBuffer[point+2]-vertexBuffer[point+0];

        if (textureBuffer.size()>0)
        {
            // UV delta
            u1 = textureBuffer[point+1]-textureBuffer[point+0];
            u2 = textureBuffer[point+2]-textureBuffer[point+0];

            float r = 1.0f / (u1.x * u2.y - u1.y * u2.x);
            tangent = (v1 * u2.y   - v2 * u1.y)*r;
            bitangent = (v2 * u1.x   - v1 * u2.x)*r;
        }
        else
        {
            // results in ugly edges
            glm::vec3 norm=glm::normalize(normalsBuffer[point]);
            tangent       =glm::normalize(v1);
            bitangent     =glm::cross(norm,tangent);
        }

        // Set the same tangent for all three vertices of the triangle.
        tangentsBuffer[point+0]=tangent;
        tangentsBuffer[point+1]=tangent;
        tangentsBuffer[point+2]=tangent;
        // Set the same tangent for all three vertices of the triangle.
        bitangentsBuffer[point+0]=bitangent;
        bitangentsBuffer[point+1]=bitangent;
        bitangentsBuffer[point+2]=bitangent;
    }
}

//Model
Model::Model()
{

}
Model::~Model()
{
    release();
}
void Model::release()
{
	// release all objects
	SharedModel* sh_model=isModelShared(modelFileName);
	if (sh_model)
    {
        if ( --(sh_model->used) <= 0  )
        {
            for (unsigned int object_index=0; object_index<objects.size(); object_index++)
            {
                objects[object_index].release();
                delete sh_model->sh_objects[object_index];
            }
            // free shared data
            delete sh_model;
            // remove shared pointer from vector
            sh_models.erase(  sh_models.begin() + sharedModelIndex(sh_model) );
        }
        for (vector<Material*>::iterator material = materials.begin(); material != materials.end(); ++material)
        {
            deleteMap((*material)->mAmbient);
            deleteMap((*material)->mDiffuse);
            deleteMap((*material)->mNormal);
            deleteMap((*material)->mSpecular);
            deleteMap((*material)->mEmission);
            deleteMap((*material)->mDepth);
            delete (*material);
        }
    }
}
bool Model::load(std::string fileName, std::string mFileName)
{
    modelFileName=fileName;
    materialFileName=mFileName;
    return load();
}
bool Model::load()
{
    bool result=false;
    SharedModel* sh_model;
    modelFilePath=modelFileName.substr(0,modelFileName.find_last_of("/\\")+1);

	if ( (sh_model=isModelShared(modelFileName)) )
    {
        // Model Mesh Specs
        radius=sh_model->radius;
        center=sh_model->center;
        minPoint=sh_model->minPoint;
        maxPoint=sh_model->maxPoint;

        // Model Materials
        if (materialFileName.length()==0)
        {
            materialFileName=sh_model->materialFileName;
        }
        loadMaterials_mtl(materialFileName);

        // Model Objects
        objects.resize(sh_model->sh_objects.size());
        for (unsigned int obj=0; obj<sh_model->sh_objects.size() ; obj++)
        {
            objects[obj].materialName=sh_model->sh_objects[obj]->materialName;
            objects[obj].numVertices =sh_model->sh_objects[obj]->numVertices;
            objects[obj].vao[0]      =sh_model->sh_objects[obj]->vao[0];
            memcpy( objects[obj].vbo, sh_model->sh_objects[obj]->vbo, 5*sizeof(GLuint) ); // copy VBO id buffer
        }
        result = true;
    }
    else // new model
    {
        // model to shared_models
        sh_model=new SharedModel();
        sh_models.push_back(sh_model);
        sh_model->modelFileName=modelFileName;

        std::string extention=modelFileName.substr(modelFileName.find_last_of(".")+1);
        if ( (!extention.compare("obj")) && loadModel_obj() )
        {
            // shared model data continued
            sh_model->materialFileName=materialFileName;
            sh_model->center=center;
            sh_model->radius=radius;
            sh_model->minPoint=minPoint;
            sh_model->maxPoint=maxPoint;

            for (unsigned int object=0; object < objects.size(); ++object)
            {
                objects[object].initBuffers();

                // shared model data continued
                SharedObject* sh_object=new SharedObject();
                sh_model->sh_objects.push_back(sh_object);

                sh_model->sh_objects[object]->materialName=objects[object].materialName;
                sh_model->sh_objects[object]->numVertices =objects[object].numVertices;
                sh_model->sh_objects[object]->vao[0]      =objects[object].vao[0];
                memcpy(sh_model->sh_objects[object]->vbo, objects[object].vbo, 5*sizeof(GLuint) ); // copy VBO id buffer
            }
            result = true;
        }
    }

    if (result)
    {
        for (vector<Object>::iterator object = this->objects.begin(); object != this->objects.end(); ++object)
        {
            // initialize Material of object
            for (vector<Material*>::iterator mat=this->materials.begin(); mat!= this->materials.end(); ++mat)
            {
                if (!((*mat)->name.compare(object->materialName)))
                {
                    object->material=(*mat);
                    break;
                }
                else if (!((*mat)->name.compare("")))
                {
                    object->material=*(materials.begin());
                }
            }
        }

        sh_model->used++;
        parsed=PARSING_SUCCEEDED;
        // enable model to be rendered
        shown=true;
    }
    else
    {
        parsed=PARSING_FAILED;
    }

	return result;
}
Material* Model::getMaterial(string materialName)
{
    for (vector<Material*>::iterator mat=this->materials.begin(); mat!= this->materials.end(); ++mat)
    {
        if (!((*mat)->name.compare(materialName)))
        {
            return (*mat);
        }
    }
    cout<<materialName<<" material Not found in Model";
    return materials[0]; // use first material instead

}
Object* Model::newObject()
{
    objects.push_back(*(new Object()));
    return &(objects.back());
}
void Model::delObject(int id)
{
	if ((unsigned int) id<this->objects.size())
	{
		this->objects[id].release();
		this->objects.erase(this->objects.begin()+id);
	}

}
/**
loadModel_obj()
#params:
    None
#ret val:
	bool: return true if file is successfully loaded, otherwise return false
#specs:
	currently supporting triangulated obj files only
	currently supporting tags:		v,vn,vt,f,o,g,mtllib.usemtl

*/
bool Model::loadModel_obj()
{
	ifstream  modelFile;
	std::string    line;
    Object*    object=0;

	std::vector<glm::vec3> vertices;	// hold vertices of an object
	std::vector<glm::vec3> normals;	    // hold normals  of an object
	std::vector<glm::vec2> textures;	// hold textures of an object
    float minX= 1000,minY= 1000,minZ= 1000;
    float maxX=-1000,maxY=-1000,maxZ=-1000;

	modelFile.open(modelFileName.c_str());
	if (!modelFile.is_open())
	{
		std::cout<<modelFileName.c_str()<<" Not found or could not be opened\n";
		return false;
	}

    while (getline(modelFile,line))
    {
        stringstream stream(line);	// converts each line to string stream to be scanned word by word
        string                tag;	// string holding scanned words   from string stream
        float 		     f1,f2,f3;	// floats holding scanned numbers from string stream
        bool vt_exists;
        bool vn_exists;
        stream>>tag;				// get first word in line

        // Currently supported tags   : v, vn, vt, f, o, g, mtllib, usemtl
        if (!tag.compare("v"))
        {
            // EX: v x y z
            stream>>f1>>f2>>f3;
            vertices.push_back(glm::vec3(f1,f2,f3));
            // calculate min/max for center and radius calculation
            minX=minX>f1? f1:minX;
            minY=minY>f2? f2:minY;
            minZ=minZ>f3? f3:minZ;
            maxX=maxX<f1? f1:maxX;
            maxY=maxY<f2? f2:maxY;
            maxZ=maxZ<f3? f3:maxZ;
        }
        else if (!tag.compare("vn"))
        {
            // EX: vn x y z
            stream>>f1>>f2>>f3;
            normals.push_back(glm::vec3(f1,f2,f3));
            vn_exists=true;
        }
        else if (!tag.compare("vt"))
        {
            // EX: vt x y
            stream>>f1>>f2;
            f2=1-f2; // invert v coordinate to match
            textures.push_back(glm::vec2(f1,f2));
            vt_exists=true;
        }
        else if (!tag.compare("f"))
        {
            // EX: f v1		 	v2		   v3
            // EX: f v1/vt1 	v2/vt2	   v3/vt3
            // EX: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
            // EX: f v1//n1 	v2//n2	   v3//n3
            float v1=0,v2=0,v3=0;
            float t1=0,t2=0,t3=0;
            float n1=0,n2=0,n3=0;

            if (vt_exists & vn_exists)
            {
                // EX: f v1/vt1/vn1 v2/vt2/vn2 v3/vt3/vn3
                sscanf(line.c_str(),"f %f/%f/%f %f/%f/%f %f/%f/%f", &v1,&t1,&n1, &v2,&t2,&n2, &v3,&t3,&n3 );
            }
            else if (vn_exists)
            {
                // EX: f v1//n1 v2//n2 v3//n3
                sscanf(line.c_str(),"f %f//%f %f//%f %f//%f", &v1,&n1, &v2,&n2, &v3,&n3 );
            }
            else if (vt_exists)
            {
                // EX: f v1/t1 v2/t2 v3/t3
                sscanf(line.c_str(),"f %f/%f %f/%f %f/%f", &v1,&t1, &v2,&t2, &v3,&t3 );
            }
            else
            {
                // EX: f v1 v2 v3
                sscanf(line.c_str(),"f %f %f %f", &v1, &v2, &v3 );
            }

            if (v3)
            {
                object->vertexBuffer.push_back(vertices[v1-1]);
                object->vertexBuffer.push_back(vertices[v2-1]);
                object->vertexBuffer.push_back(vertices[v3-1]);
            }
            if (t3)
            {
                object->textureBuffer.push_back(textures[t1-1]);
                object->textureBuffer.push_back(textures[t2-1]);
                object->textureBuffer.push_back(textures[t3-1]);
            }
            if (n3)
            {
                object->normalsBuffer.push_back(normals[n1-1]);
                object->normalsBuffer.push_back(normals[n2-1]);
                object->normalsBuffer.push_back(normals[n3-1]);
            }
        }
        else if ((!tag.compare("o")) || (!tag.compare("g")))
        {
            // EX: o middlePart
            string name;
            stream>>name;
            object=newObject();
            object->name=name;
            vt_exists=false;
            vn_exists=false;
        }
        else if (!tag.compare("mtllib"))
        {
            if (materialFileName.length()==0) // skip if an other material file was already specified
            {
                materialFileName=modelFilePath+line.substr(line.find_first_of(" ")+1);
            }
            loadMaterials_mtl(materialFileName);
        }
        else if (!tag.compare("usemtl"))
        {
            // add new object
            if (object->vertexBuffer.size())
            {
                string name=object->name;
                object=newObject();
                object->name=name;
            }

            stream>>object->materialName;
            // search if material already exists
            bool found=false;
            for (vector<Material*>::iterator mat=materials.begin(); mat!= materials.end(); ++mat)
            {
                if (!((*mat)->name.compare(object->materialName)))
                {
                    found=true;
                    break;
                }
            }
            // add new material
            if (!found)
            {
                Material* mat=new Material();
                mat->name=object->materialName;
                materials.push_back(mat);
            }
        }

    }
    radius=( (maxX-minX)/3 + (maxY-minY)/3 + (maxZ-minZ)/3 )/2;
    center.x=(minX+maxX)/2;
    center.y=(minY+maxY)/2;
    center.z=(minZ+maxZ)/2;
    minPoint=glm::vec3(minX,minY,minZ);
    maxPoint=glm::vec3(maxX,maxY,maxZ);
    modelFile.close();
    return true;

}
bool Model::loadMaterials_mtl(string materialFileName)
{
	ifstream  materialFile;
	std::string    line;
    Material* 	 material;


	materialFile.open(materialFileName.c_str());
	if (!materialFile.is_open())
	{
		std::cout<<materialFileName.c_str()<<" Not found or could not be opened\n";
		// create a dummy material to be used by model
        material=new Material();
        material->name="None";
        materials.push_back(material);

		return false;
	}

    while (getline(materialFile,line))
    {
        stringstream stream(line);	// converts each line to string stream to be scanned word by word
        string                tag;	// string holding scanned words   from string stream
        float 		     f1,f2,f3;	// floats holding scanned numbers from string stream

        stream>>tag;				// get first word in line

        // Currently supported tags :
        // newmtl , Ka , Kd , Ks , Ke , d ,Tr
        // map_Ka , map_Kd , map_Ks , map_Ke , map_Bump , map_Depth

        if (!tag.compare("newmtl"))
        {
            material=new Material();

            stream>>material->name;
            materials.push_back(material);
        }
        else if (!tag.compare("Ka"))
        {
            stream>>f1>>f2>>f3;
            material->ambient = glm::vec3(f1,f2,f3);
        }
        else if (!tag.compare("Kd"))
        {
            stream>>f1>>f2>>f3;
            material->diffuse = glm::vec3(f1,f2,f3);
        }
        else if (!tag.compare("Ks"))
        {
            stream>>f1>>f2>>f3;
            material->specular = glm::vec3(f1,f2,f3);
        }
        else if (!tag.compare("Ke"))
        {
            stream>>f1>>f2>>f3;
            material->emission = glm::vec3(f1,f2,f3);
        }
        else if (!tag.compare("d"))
        {
            stream>>f1;
            material->depth = f1;
        }
        else if (!tag.compare("Tr"))
        {
            stream>>f1;
            material->depth = 1-f1;
        }
        else if (!tag.compare("map_Ka"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mAmbient=loadMap(name);
        }
        else if (!tag.compare("map_Kd"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mDiffuse=loadMap(name);
        }
        else if (!tag.compare("map_Ks"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mSpecular=loadMap(name);
        }
        else if (!tag.compare("map_Ke"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mEmission=loadMap(name);
        }
        else if (!tag.compare("map_Bump"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mNormal=loadMap(name);
        }
        else if (!tag.compare("map_d"))
        {
            std::string name=line.substr(line.find_first_of(" ")+1);
            if (name.at(1)!=':') // name is not full path
            {
                name=modelFilePath+name;
            }
            material->mDepth=loadMap(name);
        }
    }
    materialFile.close();
    return true;
}
bool Model::updateMaterials(std::string materialFileName)
{
    vector<Material*> old_materials(materials);
    materials.clear();
    if (!loadMaterials_mtl(materialFileName))
    {
        materials.swap(old_materials);
        return false;
    }
    this->materialFileName=materialFileName;

    for (vector<Object>::iterator object = this->objects.begin(); object != this->objects.end(); ++object)
    {
        // initialize Material of object
        for (vector<Material*>::iterator mat=this->materials.begin(); mat!= this->materials.end(); ++mat)
        {
            if (!((*mat)->name.compare(object->materialName)))
            {
                object->material=(*mat);
                break;
            }
            else if (!((*mat)->name.compare("")))
            {
                object->material=*(materials.begin());
            }
        }
    }
	for (vector<Material*>::iterator material = old_materials.begin(); material != old_materials.end(); ++material)
    {
        deleteMap((*material)->mAmbient);
        deleteMap((*material)->mDiffuse);
        deleteMap((*material)->mNormal);
        deleteMap((*material)->mSpecular);
        deleteMap((*material)->mEmission);
        deleteMap((*material)->mDepth);
        delete (*material);
    }

    return true;
}
static SharedModel* isModelShared(std::string path)
{
    for(unsigned int model_index=0;model_index<sh_models.size();model_index++)
    {
        if (!sh_models[model_index]->modelFileName.compare(path) )
        {
            return sh_models[model_index];
        }
    }
    return 0;
}
static int sharedModelIndex(SharedModel* sh_model)
{
    for(unsigned int model_index=0;model_index<sh_models.size();model_index++)
    {
        if (sh_models[model_index]== sh_model )
        {
            return model_index;
        }
    }

    // default value
    return 0;
}
