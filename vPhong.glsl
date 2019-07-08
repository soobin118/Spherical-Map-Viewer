#version 330

in  vec4 vPosition;
in  vec4 vColor;
in  vec3 vNormal; 

out vec3 N3; 
out vec3 L3; 
out vec3 V3;  

out	vec3 RP3;	//Reflection에 사용할 position
out vec3 RN3;	//Reflection에 사용할 normal - 둘다 world coord

uniform mat4 uModelMat;  
uniform mat4 uProjMat; 
uniform vec4 uLPos; 
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 
uniform float uShininess; 

void main()
{
	gl_Position  = uProjMat*uModelMat*vPosition;
	gl_Position *= vec4(1,1,-1,1);
   
	vec4 N = uModelMat*vec4(vNormal,0); 
	vec4 L = uLPos - uModelMat*vPosition; 
	vec4 V = vec4(0, 0, 0, 1) - uModelMat*vPosition; 

	N3 = normalize(N.xyz); 
	L3 = normalize(L.xyz); 
	V3 = normalize(V.xyz); 

	N = uModelMat*vec4(vNormal,0); 
	vec4 P = uModelMat*vPosition;
	RN3 =normalize(N.xyz);
	RP3 = P.xyz;	// 좌표니까 normalize하면 안됨!

}
