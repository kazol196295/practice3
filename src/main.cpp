//
//  main.cpp
//  3D Object Drawing with Axes and Camera Movement
//
//  Created by Nazirul Hasan on 4/9/23.
//  modified by Badiuzzaman on 3/11/24.
//  Modified by Assistant to add dynamic camera movement, rotation, and 3D Axis Drawing on 11/19/24.
//

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "basic_camera.h"

#include <iostream>
#include <vector>
#include <cmath>

using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void generateSphereVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, int rings, float radius);
void drawSphere(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float scX, float scY, float scZ, float radius, glm::vec4 color);
void drawCone(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans, float posX, float posY, float posZ, float rotX, float rotY, float rotZ, float scX, float scY, float scZ, float height, float radius, glm::vec4 color);
void generateConeVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, float height, float radius);

// draw object functions
void drawCube(Shader shaderProgram, unsigned int VAO,
              glm::mat4 parentTrans,
              float posX = 0.0f, float posY = 0.0f, float posZ = 0.0f,
              float rotX = 0.0f, float rotY = 0.0f, float rotZ = 0.0f,
              float scX = 1.0f, float scY = 1.0f, float scZ = 1.0f,
              glm::vec4 color = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f)); // Default color is green

// Genearate Cylinder Vertices shadowing
void generateCylinderVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, float height, float radius);

// draw Cylinder shadow parameter
void drawCylinder(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                  float posX, float posY, float posZ,
                  float rotX, float rotY, float rotZ,
                  float scX, float scY, float scZ,
                  float height, float radius, glm::vec4 color);
// define callback function
void window_close_callback(GLFWwindow *window)
{
    glfwSetWindowShouldClose(window, GLFW_TRUE);
}

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// modelling transform
float rotateAngle_X = 0.0;
float rotateAngle_Y = 0.0;
float rotateAngle_Z = 0.0;
float translate_X = 0.0;
float translate_Y = 0.0;
float translate_Z = 0.0;
float scale_X = 1.0;
float scale_Y = 1.0;
float scale_Z = 1.0;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

BasicCamera basic_camera(3.0f, 3.0f, 3.0f, 0.0f, 0.0f, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));

// timing
float deltaTime = 0.0f; // time between current frame and last frame
float lastFrame = 0.0f;
float fanRotateAngle_Y = 0.0f;
bool isFanRotating = false;

struct PointLight
{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight
{
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLight
{
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float cutOff;
    float outerCutOff;
};

struct EmissiveLight
{
    glm::vec3 color;
};

// Light toggle flags
bool directionalLightOn = true;
bool pointLight1On = true;
bool pointLight2On = true;
bool spotLightOn = true;
bool emissiveLightOn = true;
bool ambientOn = true;
bool diffuseOn = true;
bool specularOn = true;

// Define lights
DirectionalLight directionalLight = {
    glm::vec3(-0.0f, -0.0f, -1.0f), // Direction of the light
    glm::vec3(0.1f, 0.1f, 0.1f),    // Ambient color
    glm::vec3(0.8f, 0.8f, 0.8f),    // Diffuse color
    glm::vec3(1.0f, 1.0f, 1.0f)     // Specular color
};

PointLight pointLight1 = {
    glm::vec3(-2.0f, 0.0f, 2.0f), // Position
    glm::vec3(0.1f, 0.1f, 0.1f),  // Ambient color
    glm::vec3(0.8f, 0.8f, 0.8f),  // Diffuse color
    glm::vec3(1.0f, 1.0f, 1.0f),  // Specular color
    1.0f, 0.09f, 0.032f           // Attenuation factors
};

PointLight pointLight2 = {
    glm::vec3(0.0f, 2.2f, -2.9f),
    glm::vec3(0.1f, 0.1f, 0.1f),
    glm::vec3(0.8f, 0.8f, 0.8f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    1.0f, 0.09f, 0.032f};

SpotLight spotLight = {
    glm::vec3(-2.0f, 0.0f, 1.7f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.1f, 0.1f, 0.1f),
    glm::vec3(0.8f, 0.8f, 0.8f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::cos(glm::radians(12.5f)),
    glm::cos(glm::radians(17.5f))};

EmissiveLight emissiveLight = {
    glm::vec3(1.0f, 1.0f, 1.0f) // White light
};

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CSE 4208: Computer Graphics Laboratory", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowCloseCallback(window, window_close_callback); // Set the window close callback function

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader program
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes for cube
    float cube_vertices[] = {
        // positions          // colors
        0.0f, 0.0f, 0.0f, 0.3f, 0.8f, 0.5f,
        0.5f, 0.0f, 0.0f, 0.5f, 0.4f, 0.3f,
        0.5f, 0.5f, 0.0f, 0.2f, 0.7f, 0.3f,
        0.0f, 0.5f, 0.0f, 0.6f, 0.2f, 0.8f,
        0.0f, 0.0f, 0.5f, 0.8f, 0.3f, 0.6f,
        0.5f, 0.0f, 0.5f, 0.4f, 0.4f, 0.8f,
        0.5f, 0.5f, 0.5f, 0.2f, 0.3f, 0.6f,
        0.0f, 0.5f, 0.5f, 0.7f, 0.5f, 0.4f};
    unsigned int cube_indices[] = {
        0, 3, 2,
        2, 1, 0,

        1, 2, 6,
        6, 5, 1,

        5, 6, 7,
        7, 4, 5,

        4, 7, 3,
        3, 0, 4,

        6, 2, 3,
        3, 7, 6,

        1, 5, 4,
        4, 0, 1};

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // update fan rotation if it's rotating
        if (isFanRotating)
        {
            fanRotateAngle_Y += 200.0f * deltaTime; // Adjust rotation speed as needed
            if (fanRotateAngle_Y > 360.0f)
                fanRotateAngle_Y -= 360.0f;
        }

        // render
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // pass projection matrix to shader
        glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);

        // camera/view transformation
        glm::mat4 view = basic_camera.createViewMatrix();
        ourShader.setMat4("view", view);

        std::vector<float> cylinderVertices;
        std::vector<unsigned int> cylinderIndices;
        int segments = 36;
        float height = 0.9f;
        float radius = 0.5f;

        // Generate cylinder vertices and indices
        generateCylinderVertices(cylinderVertices, cylinderIndices, segments, height, radius);

        // Create buffers and arrays
        unsigned int cylinderVAO, cylinderVBO, cylinderEBO;
        glGenVertexArrays(1, &cylinderVAO);
        glGenBuffers(1, &cylinderVBO);
        glGenBuffers(1, &cylinderEBO);

        glBindVertexArray(cylinderVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cylinderVBO);
        glBufferData(GL_ARRAY_BUFFER, cylinderVertices.size() * sizeof(float), &cylinderVertices[0], GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cylinderEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, cylinderIndices.size() * sizeof(unsigned int), &cylinderIndices[0], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);

        // color attribute
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glm::mat4 parentTrans = glm::mat4(1.0f);

        // Apply a translation to move the entire table
        parentTrans = glm::translate(parentTrans, glm::vec3(translate_X, translate_Y, translate_Z));

        // Apply rotation around the X-axis
        parentTrans = glm::rotate(parentTrans, glm::radians(rotateAngle_X), glm::vec3(1.0f, 0.0f, 0.0f));

        // Apply rotation around the Y-axis
        parentTrans = glm::rotate(parentTrans, glm::radians(rotateAngle_Y), glm::vec3(0.0f, 1.0f, 0.0f));

        // Apply rotation around the Z-axis
        parentTrans = glm::rotate(parentTrans, glm::radians(rotateAngle_Z), glm::vec3(0.0f, 0.0f, 1.0f));

        // Drawing Table
        // drawCube(ourShader, VAO, parentTrans, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 3.0f, 0.2f, 3.0f, glm::vec4(0.72f, 0.52f, 0.04f, 1.0f)); // wooden surface
        drawCylinder(ourShader, cylinderVAO, parentTrans,
                     0.5f, 0.5f, 0.5f,  // position
                     0.0f, 0.0f, 0.0f,  // rotation
                     1.9f, 0.04f, 1.9f, // scale
                     0.8f, 0.01f,
                     glm::vec4(0.72f, 0.52f, 0.04f, 1.0f));

        // cylindrical leg
        drawCylinder(ourShader, cylinderVAO, parentTrans,
                     0.0f, 0.36f, 0.0f, // position
                     0.0f, 0.0f, 0.0f,  // rotation
                     0.3f, 0.56f, 0.3f, // scale
                     0.5f, 0.01f,
                     glm::vec4(0.6f, 0.6f, 0.6f, 1.0f));

        // Chair 1 (Front)
        drawCube(ourShader, VAO, parentTrans, 0.0f, 0.25f, 0.9f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 1.0f, glm::vec4(0.54f, 0.27f, 0.07f, 1.0f)); // wooden seat
        drawCube(ourShader, VAO, parentTrans, -0.2f, 0.1f, 0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, 0.2f, 0.1f, 0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, -0.2f, 0.1f, 1.1f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, 0.2f, 0.1f, 1.1f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, 0.0f, 0.4f, 1.13f, 0.0f, 90.0f, 0.0f, 0.1f, 0.8f, 1.0f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f)); // backrest

        // Chair 2 (Back)
        drawCube(ourShader, VAO, parentTrans, 0.0f, 0.25f, -0.9f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 1.0f, glm::vec4(0.54f, 0.27f, 0.07f, 1.0f));  // wooden seat
        drawCube(ourShader, VAO, parentTrans, -0.2f, 0.1f, -0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, 0.2f, 0.1f, -0.7f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));    // leg
        drawCube(ourShader, VAO, parentTrans, 0.2f, 0.1f, -1.1f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));    // leg
        drawCube(ourShader, VAO, parentTrans, -0.2f, 0.1f, -1.1f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, 0.0f, 0.4f, -1.13f, 0.0f, -90.0f, 0.0f, 0.1f, 0.8f, 1.0f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f)); // backrest

        // Chair 3 (Right)
        drawCube(ourShader, VAO, parentTrans, 0.9f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 1.0f, glm::vec4(0.54f, 0.27f, 0.07f, 1.0f)); // wooden seat
        drawCube(ourShader, VAO, parentTrans, 0.7f, 0.1f, -0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, 0.7f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, 1.1f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, 1.1f, 0.1f, -0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, 1.13f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.8f, 1.0f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // backrest

        // Chair 4 (Left)
        drawCube(ourShader, VAO, parentTrans, -0.9f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.2f, 1.0f, glm::vec4(0.54f, 0.27f, 0.07f, 1.0f)); // wooden seat
        drawCube(ourShader, VAO, parentTrans, -0.7f, 0.1f, -0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, -0.7f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, -1.1f, 0.1f, 0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));   // leg
        drawCube(ourShader, VAO, parentTrans, -1.1f, 0.1f, -0.2f, 0.0f, 0.0f, 0.0f, 0.2f, 0.5f, 0.2f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // leg
        drawCube(ourShader, VAO, parentTrans, -1.13f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.8f, 1.0f, glm::vec4(0.4f, 0.26f, 0.13f, 1.0f));  // backrest

        // Drawing floor
        drawCube(ourShader, VAO, parentTrans, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 12.0, 0.05, 12.0, glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

        // Drawing walls
        drawCube(ourShader, VAO, parentTrans, -3.0, 1.5, 0.0f, 0.0f, 0.0f, 0.0f, 0.05, 6.0, 12.0, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // wall
        drawCube(ourShader, VAO, parentTrans, 3.0, 1.5, 0.0f, 0.0f, 0.0f, 0.0f, 0.05, 6.0, 12.0, glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));  // wall
        drawCube(ourShader, VAO, parentTrans, 0.0f, 1.5, -3.0, 0.0f, 0.0f, 0.0f, 12.0, 6.0, 0.05, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)); // wall

        // Drawing ceiling
        drawCube(ourShader, VAO, parentTrans,
                 0.0f, 3.0f, 0.0f,                   // position (centered at the top of the room)
                 0.0f, 0.0f, 0.0f,                   // rotation
                 12.0f, 0.05f, 12.0f,                // scale (covering the entire room)
                 glm::vec4(0.7f, 0.7f, 0.7f, 1.0f)); // color (light gray)

        // Drawing fridge
        drawCube(ourShader, VAO, parentTrans, -2.5, 0.5, -0.5f, 0.0f, 0.0f, 0.0f, 1.3f, 2.0f, 1.3f, glm::vec4(0.8f, 0.80f, 1.0f, 1.0f));  // lower body
        drawCube(ourShader, VAO, parentTrans, -2.5, 1.25, -0.5f, 0.0f, 0.0f, 0.0f, 1.3f, 1.0f, 1.3f, glm::vec4(0.8f, 0.88f, 1.0f, 1.0f)); // upper body
        drawCube(ourShader, VAO, parentTrans, -2.15, 0.5, -0.3f, 0.0f, 0.0f, 0.0f, 0.1f, 0.7f, 0.1f, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f));  // lower handle
        drawCube(ourShader, VAO, parentTrans, -2.15, 1.25, -0.3f, 0.0f, 0.0f, 0.0f, 0.1f, 0.5f, 0.1f, glm::vec4(0.2f, 0.2f, 0.2f, 1.0f)); // lower handle

        // Draw the fan
        glm::mat4 fanTransform = glm::translate(parentTrans, glm::vec3(0.0f, 2.4f, 0.0f));                     // Move fan above the floor
        fanTransform = glm::rotate(fanTransform, glm::radians(fanRotateAngle_Y), glm::vec3(0.0f, 1.0f, 0.0f)); // Rotate the fan around Y-axis

        // Fan base
        drawCylinder(ourShader, cylinderVAO, fanTransform,
                     0.0f, 0.4f, 0.0f,                   // position
                     0.0f, 0.0f, 0.0f,                   // rotation
                     0.1f, 0.08f, 0.1f,                  // scale
                     0.2f, 0.35f,                        // height, radius
                     glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // red color

        // Fan stand
        drawCube(ourShader, VAO, fanTransform,
                 -0.01f, 0.45f, 0.0f,                // position
                 0.0f, 0.0f, 0.0f,                   // rotation
                 0.05f, 0.5f, 0.05f,                 // scale
                 glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // color

        // Fan blades
        for (int i = 0; i < 4; ++i)
        {
            glm::mat4 bladeTransform = glm::rotate(fanTransform, glm::radians(90.0f * i), glm::vec3(0.0f, 1.0f, 0.0f));
            drawCube(ourShader, VAO, bladeTransform,
                     0.0f, 0.4f, 0.2f,                   // position
                     0.0f, 0.0f, 0.0f,                   // rotation
                     0.3f, 0.08f, 0.9f,                  // scale
                     glm::vec4(0.5f, 0.5f, 0.5f, 1.0f)); // color
        }

        // Draw the flat TV
        glm::mat4 tvTransform = glm::translate(parentTrans, glm::vec3(0.0f, 1.5f, -2.9f)); // Position the TV on the wall

        // TV screen
        drawCube(ourShader, VAO, tvTransform,
                 0.0f, 0.0f, 0.0f,                   // position
                 0.0f, 0.0f, 0.0f,                   // rotation
                 1.9f, 1.0f, 0.05f,                  // scale
                 glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // color

        // TV stand
        drawCube(ourShader, VAO, tvTransform,
                 0.0f, -0.55f, 0.0f,                 // position
                 0.0f, 0.0f, 0.0f,                   // rotation
                 0.2f, 0.1f, 0.05f,                  // scale
                 glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)); // color

        // Draw the table lamp
        glm::mat4 lampTransform = glm::translate(parentTrans, glm::vec3(-2.0f, 0.0f, 2.0f)); // Position the lamp on the table

        // Lamp base
        drawCube(ourShader, VAO, lampTransform,
                 0.0f, 0.1f, 0.0f,                   // position
                 0.0f, 0.0f, 0.0f,                   // rotation
                 0.2f, 0.1f, 0.2f,                   // scale
                 glm::vec4(0.3f, 0.3f, 0.3f, 1.0f)); // color (dark gray)

        // Lamp stand
        drawCylinder(ourShader, cylinderVAO, lampTransform,
                     0.0f, 0.6f, 0.0f,                                                   // position
                     0.0f, 0.0f, 0.0f,                                                   // rotation
                     0.05f, 0.7f, 0.05f,                                                 // scale
                     2.0f, 0.05f,                                                        // height, radius
                     glm::vec4(222.0f / 255.0f, 113.0f / 255.0f, 90.0f / 255.0f, 1.0f)); // color (light brown)

        // Lamp shade
        drawCylinder(ourShader, cylinderVAO, lampTransform,
                     0.0f, 0.9f, 0.0f,                   // position
                     0.0f, 0.0f, 0.0f,                   // rotation
                     0.2f, 0.3f, 0.2f,                   // scale
                     0.3f, 0.2f,                         // height, radius
                     glm::vec4(1.0f, 1.0f, 0.8f, 1.0f)); // color (light yellow)

        // Draw the tube bulb
        glm::mat4 bulbTransform = glm::translate(parentTrans, glm::vec3(0.0f, 2.2f, -2.9f)); // Position the bulb on the wall near the ceiling

        drawCylinder(ourShader, cylinderVAO, bulbTransform,
                     0.0f, 0.0f, 0.0f,                   // position
                     0.0f, 0.0f, 90.0f,                  // rotation (rotate to align with the wall)
                     0.05f, 1.0f, 0.05f,                 // scale (long and thin)
                     1.0f, 0.05f,                        // height, radius
                     glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)); // color (white)

        // Draw a sphere
        drawSphere(ourShader, VAO, parentTrans,
                   -2.0f, 0.5f, 1.0f,                  // position (adjusted y to 0.5)
                   0.0f, 0.0f, 0.0f,                   // rotation
                   1.0f, 1.0f, 1.0f,                   // scale
                   0.2f,                               // radius
                   glm::vec4(1.0f, 0.0f, 0.0f, 1.0f)); // color (red)

        // Draw a cone
        drawCone(ourShader, VAO, parentTrans,
                 -2.0f, 0.0f, 1.7f,                  // position
                 0.0f, 0.0f, 0.0f,                   // rotation
                 1.0f, 1.0f, 1.0f,                   // scale
                 0.8f,                               // height
                 0.3f,                               // radius
                 glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)); // color (green)

        // Activate the shader program
        ourShader.use();

        // Set the light properties in the shader
        ourShader.setVec3("viewPos", basic_camera.Position);         // Camera position
        ourShader.setVec3("lightPos", glm::vec3(0.0f, 2.2f, -2.9f)); // Light position
        ourShader.setVec3("viewPos", basic_camera.Position);
        ourShader.setVec3("lightColor", glm::vec3(0.5f, 0.5f, 0.5f)); // Dimmed light
        //white object color
        glm::vec3 objectColor(1.0f, 1.0f, 1.0f);
        //glm::vec3 objectColor(1.0f, 0.5f, 0.31f);
        ourShader.setVec3("objectColor", objectColor);
        // Directional Light
        ourShader.setBool("directionalLightOn", directionalLightOn);
        ourShader.setVec3("directionalLight.direction", directionalLight.direction);
        ourShader.setVec3("directionalLight.ambient", directionalLight.ambient);
        ourShader.setVec3("directionalLight.diffuse", directionalLight.diffuse);
        ourShader.setVec3("directionalLight.specular", directionalLight.specular);

        // Point Light 1
        ourShader.setBool("pointLight1On", pointLight1On);
        ourShader.setVec3("pointLight1.position", pointLight1.position);
        ourShader.setVec3("pointLight1.ambient", pointLight1.ambient);
        ourShader.setVec3("pointLight1.diffuse", pointLight1.diffuse);
        ourShader.setVec3("pointLight1.specular", pointLight1.specular);
        ourShader.setFloat("pointLight1.constant", pointLight1.constant);
        ourShader.setFloat("pointLight1.linear", pointLight1.linear);
        ourShader.setFloat("pointLight1.quadratic", pointLight1.quadratic);

        // Point Light 2
        ourShader.setBool("pointLight2On", pointLight2On);
        ourShader.setVec3("pointLight2.position", pointLight2.position);
        ourShader.setVec3("pointLight2.ambient", pointLight2.ambient);
        ourShader.setVec3("pointLight2.diffuse", pointLight2.diffuse);
        ourShader.setVec3("pointLight2.specular", pointLight2.specular);
        ourShader.setFloat("pointLight2.constant", pointLight2.constant);
        ourShader.setFloat("pointLight2.linear", pointLight2.linear);
        ourShader.setFloat("pointLight2.quadratic", pointLight2.quadratic);

        // Spotlight
        ourShader.setBool("spotLightOn", spotLightOn);
        ourShader.setVec3("spotLight.position", spotLight.position);
        ourShader.setVec3("spotLight.direction", spotLight.direction);
        ourShader.setVec3("spotLight.ambient", spotLight.ambient);
        ourShader.setVec3("spotLight.diffuse", spotLight.diffuse);
        ourShader.setVec3("spotLight.specular", spotLight.specular);
        ourShader.setFloat("spotLight.cutOff", spotLight.cutOff);
        ourShader.setFloat("spotLight.outerCutOff", spotLight.outerCutOff);

        // Emissive Light
        ourShader.setBool("emissiveLightOn", emissiveLightOn);
        ourShader.setVec3("emissiveLight.color", emissiveLight.color);

        // Set the light and material uniforms
        ourShader.setVec3("lightPos", glm::vec3(0.0f, 2.2f, -2.9f)); // Light position
        ourShader.setVec3("viewPos", basic_camera.Position);
        ourShader.setVec3("lightColor", glm::vec3(0.5f, 0.5f, 0.5f)); // Dimmed light

        // Set the model matrix for the object
        glm::mat4 model = glm::mat4(1.0f);
        ourShader.setMat4("model", model);

        // Set the other uniforms (e.g., view, projection)
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        // Swap buffers and poll IO events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // De-allocate resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}

// Process input
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        isFanRotating = !isFanRotating;

    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
        translate_Y += 0.01;
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
        translate_Y -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
        translate_X += 0.01;
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
        translate_X -= 0.01;
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
        translate_Z += 0.01;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        translate_Z -= 0.01;

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        rotateAngle_X += .2;
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        rotateAngle_Y += .2;
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        rotateAngle_Z += .2;

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('W', deltaTime); // Forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('S', deltaTime); // Backward
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('A', deltaTime); // Left
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('D', deltaTime); // Right
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('E', deltaTime); // Up
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        basic_camera.ProcessKeyboard('R', deltaTime); // Down

    // Camera rotation
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        basic_camera.ProcessRotation('P', deltaTime); // Pitch up
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        basic_camera.ProcessRotation('N', deltaTime); // Pitch down
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        basic_camera.ProcessRotation('Y', deltaTime); // Yaw left
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        basic_camera.ProcessRotation('H', deltaTime); // Yaw right
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        basic_camera.ProcessRotation('L', deltaTime); // Roll counter-clockwise
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        basic_camera.ProcessRotation('R', deltaTime); // Roll clockwise

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        directionalLightOn = !directionalLightOn;

    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        pointLight1On = !pointLight1On;

    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
        pointLight2On = !pointLight2On;

    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
        spotLightOn = !spotLightOn;

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
        ambientOn = !ambientOn;

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
        diffuseOn = !diffuseOn;

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
        specularOn = !specularOn;
}

// Framebuffer size callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// Scroll callback
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset)
{
    basic_camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// Draw Cylinder Function
void drawCylinder(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                  float posX, float posY, float posZ,
                  float rotX, float rotY, float rotZ,
                  float scX, float scY, float scZ,
                  float height, float radius, glm::vec4 color)
{
    shaderProgram.use();
    std::vector<float> cylinderVertices;
    std::vector<unsigned int> cylinderIndices;
    int segments = 36;
    generateCylinderVertices(cylinderVertices, cylinderIndices, segments, height, radius);

    // Apply transformations: translation, rotation, scaling
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    modelCentered = glm::translate(model, glm::vec3(-0.25f, -0.25f, -0.25f));

    // Set the model transformation in the shader
    shaderProgram.setMat4("model", modelCentered);

    // Use the custom color passed to the function
    shaderProgram.setVec4("color", color);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, cylinderIndices.size(), GL_UNSIGNED_INT, 0);
}

// Draw Cube Function
void drawCube(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
              float posX, float posY, float posZ,
              float rotX, float rotY, float rotZ,
              float scX, float scY, float scZ,
              glm::vec4 color)
{
    shaderProgram.use();

    // Apply transformations: translation, rotation, scaling
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    modelCentered = glm::translate(model, glm::vec3(-0.25f, -0.25f, -0.25f));

    // Set the model transformation in the shader
    shaderProgram.setMat4("model", modelCentered);

    // Use the custom color passed to the function
    shaderProgram.setVec4("color", color);

    // Draw the cube
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void generateCylinderVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, float height, float radius)
{
    // Top center vertex
    vertices.push_back(0.0f);
    vertices.push_back(height / 2.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.702f); // r
    vertices.push_back(1.0f);   // g
    vertices.push_back(1.0f);   // b

    // Bottom center vertex
    vertices.push_back(0.0f);
    vertices.push_back(-height / 2.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.702f); // r
    vertices.push_back(1.0f);   // g
    vertices.push_back(1.0f);   // b

    // Generate vertices around the top and bottom circles
    for (int i = 0; i <= segments; i++)
    {
        float angle = 2.0f * 3.1416 * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Top circle vertex
        vertices.push_back(x);
        vertices.push_back(height / 2.0f);
        vertices.push_back(z);
        vertices.push_back(0.702f); // r
        vertices.push_back(1.0f);   // g
        vertices.push_back(1.0f);   // b

        // Bottom circle vertex
        vertices.push_back(x);
        vertices.push_back(-height / 2.0f);
        vertices.push_back(z);
        vertices.push_back(0.702f); // r
        vertices.push_back(1.0f);   // g
        vertices.push_back(1.0f);   // b
    }

    // Generate indices for the top and bottom circles
    for (int i = 0; i < segments; i++)
    {
        // Top circle
        indices.push_back(0);
        indices.push_back(2 + 2 * i);
        indices.push_back(2 + 2 * ((i + 1) % segments));

        // Bottom circle
        indices.push_back(1);
        indices.push_back(3 + 2 * i);
        indices.push_back(3 + 2 * ((i + 1) % segments));

        // Side triangles
        int top1 = 2 + 2 * i;
        int top2 = 2 + 2 * ((i + 1) % segments);
        int bottom1 = top1 + 1;
        int bottom2 = top2 + 1;

        indices.push_back(top1);
        indices.push_back(bottom1);
        indices.push_back(top2);

        indices.push_back(bottom1);
        indices.push_back(bottom2);
        indices.push_back(top2);
    }
}

void generateSphereVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, int rings, float radius)
{
    for (int i = 0; i <= rings; ++i)
    {
        float theta = i * glm::pi<float>() / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for (int j = 0; j <= segments; ++j)
        {
            float phi = j * 2 * glm::pi<float>() / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            float u = 1 - (float)j / segments;
            float v = 1 - (float)i / rings;

            vertices.push_back(radius * x);
            vertices.push_back(radius * y);
            vertices.push_back(radius * z);
            vertices.push_back(u);
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < rings; ++i)
    {
        for (int j = 0; j < segments; ++j)
        {
            int first = (i * (segments + 1)) + j;
            int second = first + segments + 1;

            indices.push_back(first);
            indices.push_back(second);
            indices.push_back(first + 1);

            indices.push_back(second);
            indices.push_back(second + 1);
            indices.push_back(first + 1);
        }
    }
}

void drawSphere(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
                float posX, float posY, float posZ,
                float rotX, float rotY, float rotZ,
                float scX, float scY, float scZ,
                float radius, glm::vec4 color)
{
    shaderProgram.use();
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    int segments = 36;
    int rings = 18;
    generateSphereVertices(sphereVertices, sphereIndices, segments, rings, radius);

    unsigned int sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Apply transformations: translation, rotation, scaling
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    modelCentered = glm::translate(model, glm::vec3(-0.25f, -0.25f, -0.25f));

    // Set the model transformation in the shader
    shaderProgram.setMat4("model", modelCentered);

    // Use the custom color passed to the function
    shaderProgram.setVec4("color", color);

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

    // De-allocate resources
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
}

void generateConeVertices(std::vector<float> &vertices, std::vector<unsigned int> &indices, int segments, float height, float radius)
{
    // Generate vertices for the base
    for (int i = 0; i <= segments; ++i)
    {
        float theta = i * 2.0f * glm::pi<float>() / segments;
        float x = radius * cos(theta);
        float z = radius * sin(theta);
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Add the apex vertex
    vertices.push_back(0.0f);
    vertices.push_back(height);
    vertices.push_back(0.0f);

    // Generate indices for the base
    for (int i = 0; i < segments; ++i)
    {
        indices.push_back(i);
        indices.push_back((i + 1) % segments);
        indices.push_back(segments);
    }

    // Generate indices for the sides
    for (int i = 0; i < segments; ++i)
    {
        indices.push_back(i);
        indices.push_back((i + 1) % segments);
        indices.push_back(segments + 1);
    }
}

void drawCone(Shader shaderProgram, unsigned int VAO, glm::mat4 parentTrans,
              float posX, float posY, float posZ,
              float rotX, float rotY, float rotZ,
              float scX, float scY, float scZ,
              float height, float radius, glm::vec4 color)
{
    shaderProgram.use();
    std::vector<float> coneVertices;
    std::vector<unsigned int> coneIndices;
    int segments = 36;
    generateConeVertices(coneVertices, coneIndices, segments, height, radius);

    unsigned int coneVAO, coneVBO, coneEBO;
    glGenVertexArrays(1, &coneVAO);
    glGenBuffers(1, &coneVBO);
    glGenBuffers(1, &coneEBO);

    glBindVertexArray(coneVAO);

    glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
    glBufferData(GL_ARRAY_BUFFER, coneVertices.size() * sizeof(float), &coneVertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, coneEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, coneIndices.size() * sizeof(unsigned int), &coneIndices[0], GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Apply transformations: translation, rotation, scaling
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, model, modelCentered;
    translateMatrix = glm::translate(parentTrans, glm::vec3(posX, posY, posZ));
    rotateXMatrix = glm::rotate(translateMatrix, glm::radians(rotX), glm::vec3(1.0f, 0.0f, 0.0f));
    rotateYMatrix = glm::rotate(rotateXMatrix, glm::radians(rotY), glm::vec3(0.0f, 1.0f, 0.0f));
    rotateZMatrix = glm::rotate(rotateYMatrix, glm::radians(rotZ), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
    modelCentered = glm::translate(model, glm::vec3(-0.25f, -0.25f, -0.25f));

    // Set the model transformation in the shader
    shaderProgram.setMat4("model", modelCentered);

    // Use the custom color passed to the function
    shaderProgram.setVec4("color", color);

    glBindVertexArray(coneVAO);
    glDrawElements(GL_TRIANGLES, coneIndices.size(), GL_UNSIGNED_INT, 0);

    // De-allocate resources
    glDeleteVertexArrays(1, &coneVAO);
    glDeleteBuffers(1, &coneVBO);
    glDeleteBuffers(1, &coneEBO);
}