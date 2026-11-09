#include <iostream>
#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 700;

// Camera variables
glm::vec3 camPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 targetFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Camera settings
float mouseSensitivity = 0.03f;
float cameraSpeed = 1.2f;

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    vertexColor = aPos * 0.5 + 0.5;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(vertexColor, 1.0);
}
)";

unsigned int compileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    char infoLog[512];

    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n"
                  << infoLog << std::endl;
    }

    return shader;
}

unsigned int createShaderProgram()
{
    unsigned int vertexShader =
        compileShader(GL_VERTEX_SHADER, vertexShaderSource);

    unsigned int fragmentShader =
        compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    unsigned int program = glCreateProgram();

    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    int success;
    char infoLog[512];

    glGetProgramiv(program, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader linking error:\n"
                  << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Keyboard camera movement
void processInput(GLFWwindow* window, float dt)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed = cameraSpeed * dt;

    // Ctrl = faster movement
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed *= 2.0f;

    // Forward / backward
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += camFront * speed;

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= camFront * speed;

    // Left / right
    glm::vec3 right =
        glm::normalize(glm::cross(camFront, camUp));

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= right * speed;

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += right * speed;

    // Up / down
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos += camUp * speed;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camPos -= camUp * speed;
}

// Mouse camera rotation
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = static_cast<float>(xpos);
        lastY = static_cast<float>(ypos);
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos) - lastX;
    float yoffset = lastY - static_cast<float>(ypos);

    lastX = static_cast<float>(xpos);
    lastY = static_cast<float>(ypos);

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Prevent camera flipping
    if (pitch > 89.0f)
        pitch = 89.0f;

    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;

    direction.x =
        cos(glm::radians(yaw)) * cos(glm::radians(pitch));

    direction.y = sin(glm::radians(pitch));

    direction.z =
        sin(glm::radians(yaw)) * cos(glm::radians(pitch));

    targetFront = glm::normalize(direction);
}

// Mouse scroll zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= static_cast<float>(yoffset);

    if (fov < 1.0f)
        fov = 1.0f;

    if (fov > 45.0f)
        fov = 45.0f;
}

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        SCR_WIDTH,
        SCR_HEIGHT,
        "Week 10 - Smooth Camera",
        nullptr,
        nullptr
    );

    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;

    if (glewInit() != GLEW_OK)
    {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Mouse settings
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    unsigned int shaderProgram = createShaderProgram();

    // Cube vertices
    float vertices[] = {
        // Back face
        -0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,
        -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,

        // Front face
        -0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f,-0.5f, 0.5f,

        // Left face
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f,-0.5f,
        -0.5f,-0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,

        // Right face
         0.5f, 0.5f, 0.5f,
         0.5f, 0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
         0.5f,-0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,

        // Bottom face
        -0.5f,-0.5f,-0.5f,
         0.5f,-0.5f,-0.5f,
         0.5f,-0.5f, 0.5f,
         0.5f,-0.5f, 0.5f,
        -0.5f,-0.5f, 0.5f,
        -0.5f,-0.5f,-0.5f,

        // Top face
        -0.5f, 0.5f,-0.5f,
         0.5f, 0.5f,-0.5f,
         0.5f, 0.5f, 0.5f,
         0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f,-0.5f
    };

    unsigned int VBO, VAO;

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

    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        3 * sizeof(float), (void*)0
    );

    glEnableVertexAttribArray(0);

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

    float lastFrame = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());

        float dt = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Prevent sudden jumps
        if (dt > 0.1f)
            dt = 0.1f;

        processInput(window, dt);

        // Smooth camera rotation
        float smoothing = 1.0f - exp(-3.0f * dt);

        camFront = glm::normalize(
            glm::mix(camFront, targetFront, smoothing)
        );

        glClearColor(0.08f, 0.08f, 0.12f, 1.0f);

        glClear(
            GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT
        );

        glUseProgram(shaderProgram);

        glm::mat4 view = glm::lookAt(
            camPos,
            camPos + camFront,
            camUp
        );

        glm::mat4 projection = glm::perspective(
            glm::radians(fov),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f,
            100.0f
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "view"),
            1,
            GL_FALSE,
            glm::value_ptr(view)
        );

        glUniformMatrix4fv(
            glGetUniformLocation(shaderProgram, "projection"),
            1,
            GL_FALSE,
            glm::value_ptr(projection)
        );

        glBindVertexArray(VAO);

        for (unsigned int i = 0; i < 10; i++)
        {
            glm::mat4 model = glm::mat4(1.0f);

            model = glm::translate(model, cubePositions[i]);

            float angle =
                20.0f * i +
                static_cast<float>(glfwGetTime()) * 15.0f;

            model = glm::rotate(
                model,
                glm::radians(angle),
                glm::vec3(1.0f, 0.3f, 0.5f)
            );

            glUniformMatrix4fv(
                glGetUniformLocation(shaderProgram, "model"),
                1,
                GL_FALSE,
                glm::value_ptr(model)
            );

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();

    return 0;
}