#version 330 core
layout (location = 0) in vec3 aPos;  // Position variable has attribute position 0
layout (location = 1) in vec3 aNormal; // Normal variable has attribute position 1

out vec3 FragPos; // Will hold the fragment position in world space
out vec3 Normal;  // Will hold the normal in world space

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0)); // Transform the vertex position to world space
    Normal = mat3(transpose(inverse(model))) * aNormal; // Transform the normal to world space
    gl_Position = projection * view * vec4(FragPos, 1.0); // Final position in clip space
}

