#version 330

in vec2 texCoord;

out vec4 fColor;

uniform sampler2D uTexture;

void main()
{
	fColor= texture2D(uTexture, texCoord);//uTex��� texture�� ���� texCoord�� ��ǥ�� �ش��ϴ� ���� ��ĥ�Ѵ�.
}
