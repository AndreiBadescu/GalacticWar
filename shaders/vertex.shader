#version 450

layout(location = 0) in vec2 vPos;
layout(location = 1) in vec2 vTex;
layout(location = 2) in float vTexID;

layout(location = 0) out vec2 fTex;
layout(location = 1) flat out float fTexID;

uniform vec2 uResolution;

float lerp(float a, float b, float t)
{
	return (1.0 - t) * a + b * t;
}

float invLerp(float a, float b, float v)
{
	return (v - a) / (b - a);
}

float remap(float iMin, float iMax, float oMin, float oMax, float v)
{
	float t = invLerp(iMin, iMax, v);
	return lerp(oMin, oMax, t);
}


// x + s(y-x) :lerp
//(s-x) / (y - x) :invLerp
void main()
{
	fTex = vTex;
	fTexID = vTexID;
	vec4 pos = vec4(vPos, 0.0, 1.0);
	pos.x = remap(0.0, uResolution.x, -1.0, 1.0, pos.x);
	pos.y = remap(0.0, uResolution.y, -1.0, 1.0, pos.y);

	gl_Position = pos;
}