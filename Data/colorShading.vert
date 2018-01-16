#version 330 core

// Input vertex data, different for all executions of this shader.
in vec3 vertexP;
in vec2 vertexUV;
in vec3 vertexN;
in vec3 vertexT;
in vec3 vertexB;
// Output data ; will be interpolated for each fragment.
out vec2 fragmentUV;
out vec3 vertexP_cameraspace;
out vec3 EyeDirection_cameraspace;
out vec3 vertexN_cameraspace;
out mat3 TBN;
// Values that stay constant for the whole mesh.
uniform mat4 MVP;
uniform mat4 V;
uniform mat4 M;
uniform mat3 N;

void main()
{
	// position of the vertex
	gl_Position =  MVP * vec4(vertexP,1);
	
	// Position of the vertex, in worldspace : M * position
	vec3 vertexP_worldspace = (M * vec4(vertexP,1)).xyz;
	
	// Vector that goes from the vertex to the camera, in camera space.
	// In camera space, the camera is at the origin (0,0,0).
	vertexP_cameraspace = ( V * M * vec4(vertexP,1)).xyz;
	EyeDirection_cameraspace = vec3(0,0,0) - vertexP_cameraspace;

	// UV of the vertex. No special space for this one.
	fragmentUV = vertexUV;
	
	// model to camera = ModelView
	     vertexN_cameraspace = normalize( N * vertexN );
	vec3 vertexT_cameraspace = normalize( N * vertexT );
	vec3 vertexB_cameraspace = normalize( N * vertexB );
	
	TBN = mat3(
		vertexT_cameraspace,
		vertexB_cameraspace,
		vertexN_cameraspace	
	); 	
	
}

