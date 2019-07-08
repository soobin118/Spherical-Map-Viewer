#include <vgl.h>
#include <InitShader.h>
#include "MyObject.h"
#include "MySphere.h"
#include "Targa.h"
#include <vec.h>
#include <mat.h>

MySphere sphere;
MyObject object;

GLuint program;
GLuint sphere_prog;	//배경 program
GLuint phong_prog;	//물체 program

char * filename1;
char * filename2;

mat4 g_Mat = mat4(1.0f);

vec3 ePos = vec3(0, 0,0);
vec3 aPos = vec3(0, 0, -1);

int objflag = 0;
float FresnelPower=10;
bool is_rotation = false;
bool is_diffuseMap = false;
float time = 0;

float g_aspect = 1;

float deltaAngle1 = 0.0f;
float deltaAngle2 = 0.0f;
float angle = 180/180.0*3.141592;
int xOrigin = -1;
const int w = 1000;
const int h = 600;

mat4 myLookAt(vec3 eye, vec3 at, vec3 up)
{
	mat4 V = mat4(1.0f);

	up = normalize(up);
	vec3 n = normalize(at - eye);
	float a = dot(up, n);
	vec3 v = normalize(up - a*n);
	vec3 w = cross(n, v);

	V[0] = vec4(w, dot(-w, eye));
	V[1] = vec4(v, dot(-v, eye));
	V[2] = vec4(-n, dot(n, eye));

	return V;
}

mat4 myOrtho(float l, float r, float b, float t, float zNear, float zFar)
{
	vec3 center = vec3((l + r) / 2, (b + t) / 2, -(zNear) / 2);
	mat4 T = Translate(-center);
	mat4 S = Scale(2 / (r - l), 2 / (t - b), -1 / (-zNear + zFar));
	mat4 V = S*T;

	return V;
}

mat4 myPerspective(float angle, float aspect, float zNear, float zFar)
{
	float rad = angle*3.141592 / 180.0f;
	mat4 V(1.0f);
	float h = 2 * zFar*tan(rad / 2);
	float w = aspect*h;
	mat4 S = Scale(2 / w, 2 / h, 1 / zFar);

	float c = -zNear / zFar;

	mat4 Mpt(1.0f);
	Mpt[2] = vec4(0, 0, 1 / (c + 1), -c / (c + 1));
	Mpt[3] = vec4(0, 0, -1, 0);


	V = Mpt*S;

	return V;

}

void myInit()

{
	sphere.Init(30, 30);
	object.Init("bunny.obj");

	char * map1 = "_spheremap.tga";
	char * map2 = "_diffusemap.tga";
	strcat(filename1, map1);
	strcat(filename2, map2);

	program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	
	phong_prog = InitShader("vPhong.glsl", "fPhong.glsl");
	glUseProgram(phong_prog);
	
	sphere_prog = InitShader("vSphere.glsl", "fSphere.glsl");
	glUseProgram(sphere_prog);

	GLuint tex[2];
	glGenTextures(2, tex);
	bool success = false;

	STGA image;

	for (int i = 0; i < 2; i++)
	{
		switch (i)
		{
		case 0:
			success = image.loadTGA(filename1);
			if (success == false)
				exit(1);
			glActiveTexture(GL_TEXTURE0);
			break;
		case 1:
			success = image.loadTGA(filename2);
			if(success == false)
				exit(1);
			glActiveTexture(GL_TEXTURE1);
			break;
		default:
			break;
		}
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.width, image.height, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	image.destroy();
}

void display()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	mat4 ViewMat = myLookAt(ePos, aPos, vec3(0, 1, 0));
	mat4 ProjMat = myPerspective(45, g_aspect, 0.01, 10.0f);

	g_Mat = ProjMat*ViewMat;

	// 1. Define Light Properties
	// 
	vec4 lPos = vec4(2 , 2 , 2 , 1);
	vec4 lAmb = vec4(0.5, 0.5, 0.5, 1);
	vec4 lDif = vec4(1, 1, 1, 1);
	vec4 lSpc = lDif;

	// 2. Define Material Properties
	//
	vec4 mAmb = vec4(0.3, 0.3, 0.3, 1);
	vec4 mDif = vec4(0.8, 0.8, 0.8, 1);
	vec4 mSpc = vec4(0.3, 0.3, 0.3, 1);
	float mShiny = 50;										//1~100;

															// I = lAmb*mAmb + lDif*mDif*(N·L) + lSpc*mSpc*(V·R)^n; 
	vec4 amb = lAmb*mAmb;
	vec4 dif = lDif*mDif;
	vec4 spc = lSpc*mSpc;


	/*---object or sphere or nothing---*/
	mat4 ModelMat = Translate(aPos.x, aPos.y ,aPos.z)*RotateY(time)*Scale(0.3,0.3,0.3);
	glUseProgram(phong_prog);

	GLuint uModelMat = glGetUniformLocation(phong_prog, "uModelMat");
	GLuint uProjMat = glGetUniformLocation(phong_prog, "uProjMat");
	GLuint uLPos = glGetUniformLocation(phong_prog, "uLPos");
	GLuint uAmb = glGetUniformLocation(phong_prog, "uAmb");
	GLuint uDif = glGetUniformLocation(phong_prog, "uDif");
	GLuint uSpc = glGetUniformLocation(phong_prog, "uSpc");
	GLuint uEPos = glGetUniformLocation(phong_prog, "uEPos");
	GLuint uAPos = glGetUniformLocation(phong_prog, "uAPos");
	GLuint uShininess = glGetUniformLocation(phong_prog, "uShininess");
	GLuint ufresnel = glGetUniformLocation(phong_prog, "ufresnel");
	GLuint uTexture = glGetUniformLocation(phong_prog, "uTexture");
	GLuint uTexBlur = glGetUniformLocation(phong_prog, "uTexBlur");
	GLuint uDFlag = glGetUniformLocation(phong_prog, "uDFlag");

	glUniformMatrix4fv(uModelMat, 1, true, ViewMat*ModelMat);
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat);
	glUniform4f(uLPos, lPos[0], lPos[1], lPos[2], lPos[3]);
	glUniform4f(uAmb, amb[0], amb[1], amb[2], amb[3]);
	glUniform4f(uDif, dif[0], dif[1], dif[2], dif[3]);
	glUniform4f(uSpc, spc[0], spc[1], spc[2], spc[3]);
	glUniform4f(uEPos, ePos[0], ePos[1], ePos[2], 1);	//ePos 3차원벡터라서 마지막에 1 넣어줌. ->쓰는거는 fphong, vphong.glsl
	glUniform4f(uAPos, aPos[0], aPos[1], aPos[2], 1);
	glUniform1f(uShininess, mShiny);
	glUniform1f(ufresnel, FresnelPower);
	glUniform1i(uTexture, 0);
	glUniform1i(uTexBlur, 1);
	glUniform1i(uDFlag, is_diffuseMap);

	if (objflag == 1) {
		sphere.Draw(phong_prog);
	}
	else if (objflag == 2) {
		object.Draw(phong_prog);
	}
	else
	{
	}

	/*---environment---*/
	glUseProgram(sphere_prog);
	ModelMat = Scale(5, 5, 5);
	// 3. Send Uniform Variables to the shader
	uModelMat = glGetUniformLocation(sphere_prog, "uModelMat");
	uProjMat = glGetUniformLocation(sphere_prog, "uProjMat");
	uTexture = glGetUniformLocation(sphere_prog, "uTexture");

	glUniformMatrix4fv(uModelMat, 1, true, ViewMat*ModelMat);
	glUniformMatrix4fv(uProjMat, 1, true, ProjMat);
	glUniform1i(uTexture, 0);

	sphere.Draw(sphere_prog);

	Sleep(16);					// for vSync
	glutSwapBuffers();
}


void idle()
{
	if (is_rotation == true) {
		time += 1;
		Sleep(33);
	}
	glutPostRedisplay();
}


void reshape(int w, int h)
{
	glViewport(0, 0, w, h);

	g_aspect = w / float(h);
	glutPostRedisplay();
}
void myKeyboard(unsigned char c, int x, int y)
{
	switch (c)
	{
	case '1':			// Decreasing Fresnel power parameter for shading
		if (FresnelPower > 0.5) {
			FresnelPower -= 0.5;
		}
		printf("Fresnel Power = %f\n", FresnelPower);
		break;
	case '2':			// Increasing Fresnel power parameter for shading
		if (FresnelPower <= 9.5) {
			FresnelPower += 0.5;
		}
		printf("Fresnel Power = %f\n", FresnelPower);
		break;
	case '3':			// Turn on/off the diffuse map
		is_diffuseMap = !is_diffuseMap;
		if (is_diffuseMap == true)
			printf("Diffuse Light Map On\n");
		else
			printf("Diffuse Light Map Off\n");
		break;
	case 'q':case'Q':	// Toggling between Sphere Model and Bunny Model
		objflag = (objflag + 1) % 3;
		glutPostRedisplay();
		break;
	case ' ':			// start/stop rotating the model
		is_rotation = !is_rotation;
		break;
	default:
		break;
	}
}

int RL = 0;

float lastX, lastY;
float yaw = -90;
float pitch = 0.0f;

void myMouse(int button, int state, int x, int y) {
	if (state==GLUT_DOWN) {
		if (button == GLUT_LEFT_BUTTON) {
			RL = 1;
			lastX = x;
			lastY = y;
		}
		}
}
void mouseMove(int x, int y) {
	if (RL == 1) {
		float xoffset = -x + lastX;
		float yoffset = -lastY + y;
		lastX = x;
		lastY = y;

		float sensitivity = 0.05;
		xoffset *= sensitivity;
		yoffset *= sensitivity;


		yaw += xoffset;
		pitch += yoffset;

		if (pitch > 89.0f)
			pitch = 89.0f;
		if (pitch < -89.0f)
			pitch = -89.0f;

		vec3 front;
		front.x = cos(yaw / 180.0*3.141592)*cos(pitch / 180.0*3.141592);
		front.y = sin(pitch / 180.0*3.141592);
		front.z = sin(yaw / 180.0*3.141592)*cos(pitch / 180.0*3.141592);
		
		aPos = normalize(front);
	}
}
int main(int argc, char ** argv)
{
	glutInit(&argc, argv);

	printf("SIMPLE SPHERICAL MAP VIEWER\n");
	printf("Programming Assignment #5 for Advanced Real-time Computer Graphics\n");
	printf("Department of Digital Contents, Sejong University\n");
	printf("\n----------------------------------------------------------------\n");
	printf("\nLeft Mouse Button Dragging: rotating the viewpoint\n");
	printf("Right Mouse Button Dragging: zoon-in/out\n\n");
	printf("'1' key: Decreasing Fresnel power parameter for shading\n");
	printf("'2' key: Increasing Fresnel power parameter for shading\n");
	printf("'3' key: Turn on/off the diffuse map\n");
	printf("'q' key: toggling between Sphere Model and Bunny Model\n");
	printf("Spacebar: start/stop rotating the model\n");
	printf("----------------------------------------------------------------\n");
	printf("\nInput Image Group Name(ex: class1/class2 / ny / ice etc.):");

	filename1 = (char *)malloc(sizeof(char) * 20);
	filename2 = (char *)malloc(sizeof(char) * 20);
	scanf("%s", filename1);
	strcpy(filename2, filename1);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(1000, 600);

	glutCreateWindow("Homework5_16011087 이수빈");

	glewExperimental = true;
	glewInit();

	myInit();
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(myKeyboard);
	glutMotionFunc(mouseMove);
	glutMouseFunc(myMouse);
	glutMainLoop();

	return 0;
}
