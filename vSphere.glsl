#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal;
in	vec2 vTexCoord;

out vec2 texCoord;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 

void main()
{
	gl_Position  = uProjMat*uModelMat*vPosition;
	gl_Position *= vec4(1,1,-1,1);

	texCoord = vTexCoord;
}
