#version 450

layout(location = 0) in vec2 fTex;
layout(location = 1) flat in float fTexID;

layout(location = 0) out vec4 fColor;

uniform sampler2D fTextures[32];

#define Sample(x) else if (index == x)fColor = texture(fTextures[x], fTex)

void main() {
    int index = int(fTexID);
    if (index == 0) fColor = texture(fTextures[0], fTex);
    Sample(1);
    Sample(2);
    Sample(3);
    Sample(4);
    Sample(5);
    Sample(6);
    Sample(7);
    Sample(8);
    Sample(9);
    Sample(10);
    Sample(11);
    Sample(12);
    Sample(13);
    Sample(14);
    Sample(15);
    Sample(16);
    Sample(17);
    Sample(18);
    Sample(19);
    Sample(20);
    Sample(21);
    Sample(22);
    Sample(23);
    Sample(24);
    Sample(25);
    Sample(26);
    Sample(27);
    Sample(28);
    Sample(29);
    Sample(30);
    Sample(31);
}