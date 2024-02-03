#version 330

layout( location = 0 ) in vec4 inPosition;
layout( location = 1 ) in vec2 inUV;
layout( location = 2 ) in vec3 inNormal;
// macierz modelu dla calej instancji
layout( location = 3 ) in mat4 matModelInst;

layout( location = 7 ) in vec3 originalPos;

uniform mat4 matProj;
uniform mat4 matView;

out vec4 inoutPos;
out vec2 inoutUV;

uniform mat4 matModel = mat4(1.0);
uniform float xShift = 0.0f;


void main()
{
	vec4 pos = inPosition;

	float div = (xShift-originalPos.y) / 150.0f;
	float whole;
	modf(div, whole);
	float rest = (div - whole) * 150.0f;

	pos.y -= rest;
	gl_Position = matProj * matView * matModelInst * pos;

	inoutPos = inPosition;
	inoutUV = inUV;
}
