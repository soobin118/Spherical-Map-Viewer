#version 330

in vec3 N3; 
in vec3 L3; 
in vec3 V3; 

in vec3 RP3;
in vec3 RN3;

out vec4 fColor;

uniform mat4 uModelMat; 
uniform mat4 uProjMat; 
uniform vec4 uLPos; 
uniform vec4 uAmb; 
uniform vec4 uDif; 
uniform vec4 uSpc; 
uniform vec4 uEPos;
uniform vec4 uAPos;
uniform float uShininess; 
uniform float ufresnel;
uniform sampler2D uTexture;
uniform sampler2D uTexBlur;
uniform int uDFlag;

void main()
{
	vec3 N = normalize(N3); 
	vec3 L = normalize(L3); 
	vec3 V = normalize(V3); 
	vec3 H = normalize(V+L); 

    float NL = max(dot(N, L), 0); 
	float VR = pow(max(dot(H, N), 0), uShininess); 

	vec3 view_dir = RP3 - uEPos.xyz;
	vec3 normal = normalize(RN3);
	vec3 dir = reflect(view_dir, normal);
	float m = 2. * sqrt( pow( dir.x, 2. ) + pow( dir.y, 2. ) + pow( dir.z + 1., 2. ) );
	vec2 vN;
	vN.x = dir.x / m + .5;
	vN.y = dir.y / m + .5;
	vec3 base = texture(uTexture, vN).rgb;	//reflection 반사
	vec4 rfl = vec4(base, 1.);

	vec4 phong_color;
	if(uDFlag == 1){
		vec3 diffuse = texture(uTexBlur,vN).rgb;
		phong_color = vec4(diffuse, 1);
	}
	else{
		NL = max(dot(normal, L), 0); 
		phong_color = uAmb + uDif*NL;	//	물체의 색과 반사를 같이 쓰는 것.
	} 

	
	float F = 0.0;
	float ratio = F+(1.0-F)*pow((1.0+dot(view_dir, normal)), ufresnel);
	if(ufresnel>10) ratio = 0; // 10보다 클경우 반사 없음
	fColor = mix(phong_color, rfl, ratio); 
}
