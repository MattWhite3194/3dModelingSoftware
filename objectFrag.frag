#version 330 core

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;
uniform vec4 objectColor;
vec3 lightColor = vec3(1.0, 1.0, 1.0);
vec3 ambient = vec3(1.0, 1.0, 1.0);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec4 result = vec4((ambient + diffuse), 1.0) * objectColor;
    FragColor = result;
}