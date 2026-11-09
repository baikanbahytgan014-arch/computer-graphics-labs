
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Camera
glm::vec3 camPos(0.0f, 0.0f, 3.0f);
glm::vec3 camFront(0.0f, 0.0f, -1.0f);
glm::vec3 camUp(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Cube positions
glm::vec3 cubePositions[] = {
    glm::vec3( 0.0f,  0.0f,   0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f,  -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f,  -3.5f),
    glm::vec3(-1.7f,  3.0f,  -7.5f),
    glm::vec3( 1.3f, -2.0f,  -2.5f),
    glm::vec3( 1.5f,  2.0f,  -2.5f),
    glm::vec3( 1.5f,  0.2f,  -1.5f),
    glm::vec3(-1.3f,  1.0f,  -1.5f)
};

// Each cube face has its own color
float vertices[] = {
    // Back face: red
    -0.5f,-0.5f,-0.5f,  1.0f,0.0f,0.0f,
     0.5f,-0.5f,-0.5f,  1.0f,0.0f,0.0f,
     0.5f, 0.5f,-0.5f,  1.0f,0.0f,0.0f,

     0.5f, 0.5f,-0.5f,  1.0f,0.0f,0.0f,
    -0.5f, 0.5f,-0.5f,  1.0f,0.0f,0.0f,
    -0.5f,-0.5f,-0.5f,  1.0f,0.0f,0.0f,

    // Front face: green
    -0.5f,-0.5f, 0.5f,  0.0f,1.0f,0.0f,
     0.5f,-0.5f, 0.5f,  0.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f,  0.0f,1.0f,0.0f,

     0.5f, 0.5f, 0.5f,  0.0f,1.0f,0.0f,
    -0.5f, 0.5f, 0.5f,  0.0f,1.0f,0.0f,
    -0.5f,-0.5f, 0.5f,  0.0f,1.0f,0.0f,

    // Left face: blue
    -0.5f, 0.5f, 0.5f,  0.0f,0.0f,1.0f,
    -0.5f, 0.5f,-0.5f,  0.0f,0.0f,1.0f,
    -0.5f,-0.5f,-0.5f,  0.0f,0.0f,1.0f,

    -0.5f,-0.5f,-0.5f,  0.0f,0.0f,1.0f,
    -0.5f,-0.5f, 0.5f,  0.0f,0.0f,1.0f,
    -0.5f, 0.5f, 0.5f,  0.0f,0.0f,1.0f,

    // Right face: yellow
     0.5f, 0.5f, 0.5f,  1.0f,1.0f,0.0f,
     0.5f, 0.5f,-0.5f,  1.0f,1.0f,0.0f,
     0.5f,-0.5f,-0.5f,  1.0f,1.0f,0.0f,

     0.5f,-0.5f,-0.5f,  1.0f,1.0f,0.0f,
     0.5f,-0.5f, 0.5f,  1.0f,1.0f,0.0f,
     0.5f, 0.5f, 0.5f,  1.0f,1.0f,0.0f,

    // Bottom face: orange
    -0.5f,-0.5f,-0.5f,  1.0f,0.5f,0.0f,
     0.5f,-0.5f,-0.5f,  1.0f,0.5f,0.0f,
     0.5f,-0.5f, 0.5f,  1.0f,0.5f,0.0f,

     0.5f,-0.5f, 0.5f,  1.0f,0.5f,0.0f,
    -0.5f,-0.5f, 0.5f,  1.0f,0.5f,0.0f,
    -0.5f,-0.5f,-0.5f,  1.0f,0.5f,0.0f,

    // Top face: white
    -0.5f, 0.5f,-0.5f,  1.0f,1.0f,1.0f,
     0.5f, 0.5f,-0.5f,  1.0f,1.0f,1.0f,
     0.5f, 0.5f, 0.5f,  1.0f,1.0f,1.0f,

     0.5f, 0.5f, 0.5f,  1.0f,1.0f,1.0f,
    -0.5f, 0.5f, 0.5f,  1.0f,1.0f,1.0f,
    -0.5f, 0.5f,-0.5f,  1.0f,1.0f,1.0f
};

// Vertex shader
const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = aColor;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader
const char* fragmentShaderSource = R"(
#version 330 core

in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

// Compile shader
unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);

    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        char infoLog[512];

        glGetShaderInfoLog(shader, 512, nullptr, infoLog);

        std::cout << "Shader compilation error:\n"
                  << infoLog << std::endl;
    }

    return shader;
}

// Camera movement
void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = 2.5f * dt;

    // Shift = faster movement
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed *= 2.0f;

    // Forward / backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camFront * speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camFront * speed;

    // Left / right
    glm::vec3 right = glm::normalize(glm::cross(camFront, camUp));

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= right * speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += right * speed;

    // Up / down
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos.y += speed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camPos.y -= speed;
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WIDTH, HEIGHT,
        "Task 1 - LookAt Camera - RGB Cubes",
        nullptr, nullptr
    );

    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW\n";
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Create VAO and VBO
    unsigned int VAO, VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    // Position attribute
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        6 * sizeof(float),
        (void*)(3 * sizeof(float))
    );

    glEnableVertexAttribArray(1);

    // Create shaders
    unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        char infoLog[512];

        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);

        std::cout << "Shader linking error:\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();

        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        // Camera view matrix
        glm::mat4 view = glm::lookAt(
            camPos,
            camPos + camFront,
            camUp
        );

        // Perspective projection
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)WIDTH / (float)HEIGHT,
            0.1f,
            100.0f
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "view"),
            1, GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "projection"),
            1, GL_FALSE,
            glm::value_ptr(projection)
        );

        glBindVertexArray(VAO);

        // Draw 10 rotating cubes
        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model(1.0f);

            model = glm::translate(
                model,
                cubePositions[i]
            );

            // Continuous rotation
            float angle = glfwGetTime() * 40.0f + i * 20.0f;

            model = glm::rotate(
                model,
                glm::radians(angle),
                glm::vec3(1.0f, 0.3f, 0.5f)
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shaderProgram, "model"),
                1, GL_FALSE,
                glm::value_ptr(model)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(shaderProgram);

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);

    glfwTerminate();

    return 0;
}
