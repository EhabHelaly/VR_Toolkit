#version 330 core

#define LIGHT_SOURCES_MAX		 	 5	// must change the same macro in engine
#define LIGHT_SOURCE_OFF 		 	 0
#define LIGHT_SOURCE_DIRECTIONAL 	 1
#define LIGHT_SOURCE_POINT		 	 2
#define LIGHT_SOURCE_POINT_REALISTIC 3
#define LIGHT_SOURCE_SPOT		     4	// not supported yet

struct Light
{
	int type;
	float power;
	vec3 color;
	vec3 position_cameraspace;
	vec3 direction_cameraspace;
};
struct Maps
{
	sampler2D ambient;
	sampler2D diffuse;
	sampler2D normal;
	sampler2D specular;
	sampler2D emission;
	sampler2D depth;
};
struct Material
{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	vec3 Ke;
	float depth;
};

// Interpolated values from the vertex shaders
in vec2 fragmentUV;
in vec3 vertexP_cameraspace;
in vec3 EyeDirection_cameraspace;
in vec3 vertexN_cameraspace;
in mat3 TBN;
// Ouput data
out vec4 color;
// Values that stay constant for the whole mesh.
uniform int targetModel;

uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

uniform Light lights[LIGHT_SOURCES_MAX];
uniform Maps maps;
uniform Material mat;

uniform float cosTime;
uniform float exp;

void main()
{
	// light
	vec3 LightDirection_cameraspace;
	// Material properties
	vec3 normalVector;
	vec3 colorKa;
	vec3 colorKd;
	vec3 colorKs;
	vec3 colorKe;

	// diffuse
	if (textureSize(maps.diffuse,3).x >0)
	{
		colorKd = texture( maps.diffuse, fragmentUV ).rgb;
	}
	else
	{
		colorKd= mat.Kd;
	}

	// ambient
	if (textureSize(maps.ambient,3).x >0)
	{
		colorKa = texture( maps.ambient, fragmentUV ).rgb;
		
		// if no diffuse color , use ambient color as is
		if (colorKd==vec3(0,0,0))
		{
			colorKa*=20;
		}
	}
	else
	{
		colorKa = mat.Ka*0.0001 + colorKd;
	}

	// normal
	if (textureSize(maps.normal,3).x >0)
	{
		vec3 normal=texture( maps.normal, fragmentUV ).rgb;
		normal.y=1-normal.y; // invert green
		normalVector = TBN * normalize(normal*2.0 - 1.0);
	}
	else
	{
		normalVector = vertexN_cameraspace;
	}

	// specular
	if (textureSize(maps.specular,3).x >0)
	{
		colorKs= texture( maps.specular, fragmentUV ).rgb ;
	}
	else if (textureSize(maps.specular,1).x >0)
	{
		colorKs= texture( maps.specular, fragmentUV ).r *vec3(1.0,1.0,1.0);
	}
	else
	{
		colorKs=mat.Ks;
	}

	// emission
	if (textureSize(maps.emission,3).x >0)
	{
		colorKe= texture( maps.emission, fragmentUV ).rgb ;
	}
	else if (textureSize(maps.emission,1).x >0)
	{
		colorKe= texture( maps.emission, fragmentUV ).r *vec3(1.0,1.0,1.0);
	}
	else
	{
		colorKe=mat.Ke;
	}

	// depth
	if (textureSize(maps.depth,4).x >0)
	{
		color.a = texture( maps.depth, fragmentUV ).a;
	}
	else if (textureSize(maps.depth,3).x >0)
	{
		color.a = texture( maps.depth, fragmentUV ).r;
	}
	else
	{
		color.a = mat.depth;	
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	vec3 sumKd=vec3(0.0f,0.0f,0.0f);
	vec3 sumKs=vec3(0.0f,0.0f,0.0f);
	vec3 sumKe=vec3(0.0f,0.0f,0.0f);
	for (int s = 0; s < 5; ++s)
	{
		switch(lights[s].type)
		{
			case LIGHT_SOURCE_POINT:
			{
				// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
				LightDirection_cameraspace = lights[s].position_cameraspace + EyeDirection_cameraspace;	

				float distance = length( lights[s].position_cameraspace - vertexP_cameraspace );
				vec3 l = normalize(LightDirection_cameraspace);
				float cosTheta = max( dot( normalVector,l ), 0.0 );
				vec3 E = normalize(EyeDirection_cameraspace);
				vec3 R = reflect(-l,normalVector);
				float cosAlpha = max( dot( E,R ), 0.0 );

				sumKd+=lights[s].color * lights[s].power * (cosTheta/2+ 0.5)	    / pow(distance,0.5);
				sumKs+=lights[s].color * lights[s].power * pow(cosAlpha,exp)         / pow(distance,0.5);
				break;
			}
			case LIGHT_SOURCE_POINT_REALISTIC:
			{
				// Vector that goes from the vertex to the light, in camera space. M is ommited because it's identity.
				LightDirection_cameraspace = lights[s].position_cameraspace + EyeDirection_cameraspace;	

				float distance = length( lights[s].position_cameraspace - vertexP_cameraspace );
				vec3 l = normalize(LightDirection_cameraspace);
				float cosTheta = max( dot( normalVector,l ), 0.0 );
				vec3 E = normalize(EyeDirection_cameraspace);
				vec3 R = reflect(-l,normalVector);
				float cosAlpha = max( dot( E,R ), 0.0 );

				sumKd+=lights[s].color * lights[s].power *10 * (cosTheta/2+ 0.5)  / (distance*distance);
				sumKs+=lights[s].color * lights[s].power *10 * pow(cosAlpha,exp)   / (distance*distance);
				break;
			}
			case LIGHT_SOURCE_DIRECTIONAL:
			{
				vec3 l = normalize(lights[s].direction_cameraspace);
				float cosTheta = max( dot( normalVector,l ), 0.0 );
				vec3 E = normalize(EyeDirection_cameraspace);
				vec3 R = reflect(-l,normalVector);
				float cosAlpha = clamp( dot( E,R ), 0,1 );

				sumKd+=lights[s].color * lights[s].power * (cosTheta/2+ 0.5)     /100;
				sumKs+=lights[s].color * lights[s].power * (pow(cosAlpha,exp)*3) /100;
				break;
			}
			case LIGHT_SOURCE_OFF:
			{
				break;
			}
		}
		
	}
	color.rgb = 
		// Ambient : simulates indirect lighting
		colorKa * 0.05 +
		// Diffuse : "color" of the object
		colorKd * sumKd +
		// Specular : reflective highlight, like a mirror
		colorKs * sumKs +
		// Emission : emitted light bo object
		colorKe * cosTime +
		// Highlight targeted model
		targetModel * vec3(0.2,0.2,0.2);

}