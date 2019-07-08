#pragma once

#include <vgl.h>
#include <vec.h>

struct MyObjectVertex
{
	vec4 position;
	vec4 color;
	vec3 normal;
};

class MyObject
{
public:
	MyObject(void);
	~MyObject(void);

	int NumVertices;

	MyObjectVertex * Vertices;
	//GLuint Init(int la_slice, int lo_slice, vec4 color=vec4(0.5,0.5,0.5,1));
	GLuint Init(const char * filename);
	void SetPositionAndOtherAttributes(GLuint program);

	GLuint vao;
	GLuint buffer;
	bool bInitialized;

	void Draw(GLuint program);
};



MyObject::MyObject(void)
{
	bInitialized = false;
	NumVertices = 0;
	Vertices = NULL;
}

MyObject::~MyObject(void)
{
	if (Vertices != NULL)
		delete[] Vertices;
}

GLuint MyObject::Init(const char * filename)
{
	FILE * file = fopen(filename, "r");
	if (file == NULL)
	{
		printf("file not found!");
		return 0;
	}
	int numV = 0;
	int numF = 0;
	char buf[255];
	while (!feof(file)) {
		fgets(buf, 255, file);
		if (buf[0] == 'v') numV++;
		if (buf[0] == 'f') numF++;
	}
	printf("vertex : %d, face: %d \n", numV, numF);
	fclose(file);

	// The Cube should be initialized only once;
	if (bInitialized == true) return vao;

	NumVertices = numF * 3;
	Vertices = new MyObjectVertex[NumVertices];

	vec3 * pos = new vec3[numV];	// 각각 고유한 점들을 담을 버퍼
	int ** face = new int *[numF];	// 숫자 3개씩해서 면의 정보를 받을 것 이중 배열
	for (int i = 0; i < numF; i++)
		face[i] = new int[3];

	int indV = 0;
	int indF = 0;	//몇번째 V,F인지 알려주는 것

	file = fopen(filename, "r");
	while (!feof(file)) {
		fgets(buf, 255, file);
		if (buf[0] == 'v') {
			float x, y, z;
			char c;
			sscanf(buf, "%c %f %f %f", &c, &x, &y, &z);	// string에서 scanf하는 것. file에서 scanf하는 것은 fscanf
														// sscanf는 잘 읽어온 변수의 갯수를 integer로 반환한다. 이경우 4를 반환해야 맞는 것.
														// 제대로 하려면 if문으로 4를 반환하면, 이라고 해야한다.
			pos[indV] = vec3(x, y, z);
			indV++;
		}

		if (buf[0] == 'f') {
			int a, b, c;
			char cr;
			sscanf(buf, "%c %d %d %d", &cr, &a, &b, &c);
			face[indF][0] = a - 1;
			face[indF][1] = b - 1;
			face[indF][2] = c - 1;
			indF++;
		}
	}
	printf("vertex ind : %d, face ind: %d \n", indV, indF);
	fclose(file);

	// center != centroid
	// centroid = 무게중심. 평균점. 점의 평균이다. 계산은 훨씬 쉽다. 하지만 여기서 토끼같은 경우 얼굴에 점이 많고 몸통부분에는 점이 별로 없어서 무게중심이 얼굴쪽으로 쏠릴 가능성이 있다.
	// 그러므로 여기서는 center로 하겠다.
	vec3 minP = pos[0];
	vec3 maxP = pos[0];
	for (int i = 0; i < numV; i++)
	{
		if (minP.x > pos[i].x)
			minP.x = pos[i].x;
		if (minP.y > pos[i].y)
			minP.y = pos[i].y;
		if (minP.z > pos[i].z)
			minP.z = pos[i].z;

		if (maxP.x < pos[i].x)
			maxP.x = pos[i].x;
		if (maxP.y < pos[i].y)
			maxP.y = pos[i].y;
		if (maxP.z < pos[i].z)
			maxP.z = pos[i].z;

	}
	float size = length(maxP - minP);
	vec3 center = (maxP + minP) / 2.0f;

	for (int i = 0; i < numV; i++)
		pos[i] = (pos[i] - center) / size*2.0f;

	vec3 * fnormal = new vec3[numF];	// face normal을 저장할 공간
	for (int i = 0; i < numF; i++)
	{
		vec3 a = pos[face[i][0]];
		vec3 b = pos[face[i][1]];
		vec3 c = pos[face[i][2]];
		vec3 p = b - a;
		vec3 q = c - a;
		vec3 n = normalize(cross(p, q));
		fnormal[i] = n;
	}

	vec3 * vnormal = new vec3[numV];	// vertex normal을 저장할 공간
	for (int i = 0; i < numV; i++)
		vnormal[i] = vec3(0, 0, 0);

	for (int j = 0; j < numF; j++)
		for (int k = 0; k < 3; k++)
			vnormal[face[j][k]] += fnormal[j];	//vnormal에 fnormal을 더해준다. 그러면 점을 구성하고있는 모든 면의 노말이 더해질 것임.

	for (int i = 0; i < numV; i++)
		vnormal[i] = normalize(vnormal[i]);	// 크기 1로 만들어주기

	int cur = 0;
	vec4 color = vec4(1, 0.5, 0.5, 1);
	for (int i = 0; i < numF; i++)
	{
		vec3 a = pos[face[i][0]];
		vec3 b = pos[face[i][1]];
		vec3 c = pos[face[i][2]];

		vec3 p = b - a;
		vec3 q = c - a;
		vec3 n = normalize(cross(p, q));
		// phong shading
		Vertices[cur].position = a;	Vertices[cur].color = color; Vertices[cur].normal = vnormal[face[i][0]]; cur++;
		Vertices[cur].position = b;	Vertices[cur].color = color; Vertices[cur].normal = vnormal[face[i][1]]; cur++;
		Vertices[cur].position = c;	Vertices[cur].color = color; Vertices[cur].normal = vnormal[face[i][2]]; cur++;

	}


	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(MyObjectVertex)*NumVertices, Vertices, GL_STATIC_DRAW);

	bInitialized = true;
	return vao;

}

void MyObject::SetPositionAndOtherAttributes(GLuint program)
{
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjectVertex), BUFFER_OFFSET(0));

	GLuint vColor = glGetAttribLocation(program, "vColor");
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, sizeof(MyObjectVertex), BUFFER_OFFSET(sizeof(vec4)));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(MyObjectVertex), BUFFER_OFFSET(sizeof(vec4) + sizeof(vec4)));
}


void MyObject::Draw(GLuint program)
{
	if (!bInitialized) return;			// check whether it is initiazed or not. 

	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	SetPositionAndOtherAttributes(program);

	glDrawArrays(GL_TRIANGLES, 0, NumVertices);
}