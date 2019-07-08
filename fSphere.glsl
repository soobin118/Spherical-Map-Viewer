#version 330

in vec2 texCoord;

out vec4 fColor;

uniform sampler2D uTexture;

void main()
{
	fColor= texture2D(uTexture, texCoord);//uTex라는 texture에 가서 texCoord의 좌표에 해당하는 색을 색칠한다.
}
