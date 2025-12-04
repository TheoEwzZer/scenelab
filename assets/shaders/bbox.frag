#version 330 core

out vec4 FragColor;

uniform vec3 bboxColor;

void main() { FragColor = vec4(bboxColor, 1.0); }
