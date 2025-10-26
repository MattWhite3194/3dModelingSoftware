#version 330 core
out vec4 FragColor;
uniform vec4 edgeColor;

void main() {
    FragColor = edgeColor;
}