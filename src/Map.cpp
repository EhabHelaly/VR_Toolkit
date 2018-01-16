#include "Map.h"
#include <vector>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <opencv2/opencv.hpp>

/**
* Class Map *
*
* holds images loaded in world
* allow models and objects to share images without loading them again
* keeps link between image path and its ID
*/
typedef struct {
    std::string path;
    GLuint id=0;
    int type=0;
    int used=0;
}Map;

static std::vector<Map*> maps;
static Map* getMap(std::string path);

static GLuint loadTexture  (std::string textureFileName);
static GLuint loadDDS        (const char * imagepath);
static GLuint loadTexture_any(const char * imagepath);

GLuint loadMap(std::string path)
{
    int pos=0;
    // remove "../" to get real path
	while( (pos=path.find(".."))!=-1 && pos!=0 )
    {
        path= path.substr(0,(path.substr(0,pos-1)).find_last_of("/\\")+1)+ path.substr(pos+3);
    }

	// check if image is previously loaded and get its ID
	GLuint mapID=0;
    Map* map=getMap(path);

	if (map) // found
    {
        mapID=map->id;
        map->used++;
    }
	else // not found
	{
        mapID=loadTexture(path);

        // update maps
        if (mapID)
        {
            map=new Map();
            map->path=path;
            map->id=mapID;
            map->used=1;
            maps.push_back(map);
        }
	}
	// set ID of the proper map to the generated ID
    return mapID;
}
GLuint getMapID(std::string path)
{
    int pos=0;
    // remove "../" to get real path
	while( (pos=path.find(".."))!=-1 && pos!=0 )
    {
        path= path.substr(0,(path.substr(0,pos-1)).find_last_of("/\\")+1)+ path.substr(pos+3);
    }

    Map* map=getMap(path);
    return map? map->id:0;
}
void deleteMap(GLuint mapID)
{
	for (unsigned int m=0; m<maps.size();m++)
	{
		if ( ( maps[m]->id==mapID ) )
        {
            if (--(maps[m]->used) == 0)
            {
                glDeleteTextures(1,&(maps[m]->id));
                delete maps[m];
                maps.erase(maps.begin()+m);
            }
            break;
        }
	}
}
void deleteMaps()
{
	for (unsigned int m=0; m<maps.size();m++)
	{
        glDeleteTextures(1,&(maps[m]->id));
        delete maps[m];
	}
	maps.clear();
}

static Map* getMap(std::string path)
{
	for (unsigned int m=0; m<maps.size();m++)
	{
		if ( !( maps[m]->path.compare(path) ) )
        {
            return maps[m];
        }
	}
    return 0;
}

static GLuint loadTexture  (std::string textureFileName)
{
    GLuint textureID;
    std::string extention=textureFileName.substr(textureFileName.find_last_of(".")+1);
    if (!extention.compare("dds"))
    {
        textureID = loadDDS(textureFileName.c_str());
    }
    else
    {
        textureID = loadTexture_any(textureFileName.c_str());
    }
    return textureID;
}
static GLuint loadDDS(const char * imagepath)
{
#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII


	unsigned char header[124];

	FILE *fp;

	/* try to open the file */
	fp = fopen(imagepath, "rb");
	if (fp == NULL){
		printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
		return 0;
	}

	/* verify the type of file */
	char filecode[4];
	fread(filecode, 1, 4, fp);
	if (strncmp(filecode, "DDS ", 4) != 0) {
		fclose(fp);
		return 0;
	}

	/* get the surface desc */
	fread(&header, 124, 1, fp);

	unsigned int height      = *(unsigned int*)&(header[8 ]);
	unsigned int width	     = *(unsigned int*)&(header[12]);
	unsigned int linearSize	 = *(unsigned int*)&(header[16]);
	unsigned int mipMapCount = *(unsigned int*)&(header[24]);
	unsigned int fourCC      = *(unsigned int*)&(header[80]);


	unsigned char * buffer;
	unsigned int bufsize;
	/* how big is it going to be including all mipmaps? */
	bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
	buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
	fread(buffer, 1, bufsize, fp);
	/* close the file pointer */
	fclose(fp);

	unsigned int format;
	switch(fourCC)
	{
	case FOURCC_DXT1:
		format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
		break;
	case FOURCC_DXT3:
		format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
		break;
	case FOURCC_DXT5:
		format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
		break;
	default:
		free(buffer);
		return 0;
	}

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);

	unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
	unsigned int offset = 0;

	/* load the mipmaps */
	for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
	{
		unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize;
		glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
			0, size, buffer + offset);

		offset += size;
		width  /= 2;
		height /= 2;

		// Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
		if(width < 1) width = 1;
		if(height < 1) height = 1;

	}

	free(buffer);

    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

    glGenerateMipmap(GL_TEXTURE_2D);

	return textureID;


}
static GLuint loadTexture_any(const char * imagepath)
{
    cv::Mat image=cv::imread(imagepath,-1);
	if (image.empty())
    {
        printf("Failed to load texture %s\n",imagepath);
        return 0;
    }
    // resize all textures ( width and height) to a power of 2 number
    // this works best for glTexImage2D function, otherwise it would crash on some dimensions
    int dimC = 1<< (int)glm::round(log2(image.cols));
    int dimR = 1<< (int)glm::round(log2(image.rows));
    cv::resize(image,image,cv::Size(dimC,dimR));

	// Create one OpenGL texture
	GLuint textureID;
	glGenTextures(1, &textureID);

	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, textureID);

	// Give the image to OpenGL
	if      ((image.channels()==4))
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.data);
    else if (image.channels()==3)
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB , image.cols, image.rows, 0, GL_BGR , GL_UNSIGNED_BYTE, image.data);
    else if ((image.channels()==1))
        glTexImage2D(GL_TEXTURE_2D, 0, GL_R   , image.cols, image.rows, 0, GL_R   , GL_UNSIGNED_BYTE, image.data);
    else
    {
        printf("Unexpected number of channels in texture %s",imagepath);
        return 0;
    }


    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);



    glGenerateMipmap(GL_TEXTURE_2D);

	// Return the ID of the texture we just created
	return textureID;
}
