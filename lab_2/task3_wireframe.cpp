#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <iostream>

const char* vertexSrc = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentSrc = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";

int main()
{
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        800,
        600,
        "Task 3 - Wireframe",
        nullptr,
        nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (gladLoadGL(glfwGetProcAddress) == 0)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    float vertices[] =
    {
        -0.8f, -0.5f, 0.0f,
        -0.2f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f,

         0.2f, -0.5f, 0.0f,
         0.8f, -0.5f, 0.0f,
         0.5f,  0.5f, 0.0f
    };

    unsigned int vao;
    unsigned int vbo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);

    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(vertices),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        3 * sizeof(float),
        (void*)0
    );

    glEnableVertexAttribArray(0);

    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vertexSrc, nullptr);
    glCompileShader(vs);

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fragmentSrc, nullptr);
    glCompileShader(fs);

    unsigned int shader = glCreateProgram();

    glAttachShader(shader, vs);
    glAttachShader(shader, fs);

    glLinkProgram(shader);

    glDeleteShader(vs);
    glDeleteShader(fs);

    while (!glfwWindowShouldClose(window))
    {
        glClearColor(
            0.1f,
            0.1f,
            0.1f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader);

        glBindVertexArray(vao);

        glPolygonMode(
            GL_FRONT_AND_BACK,
            GL_LINE
        );

        glDrawArrays(
            GL_TRIANGLES,
            0,
            6
        );

        glPolygonMode(
            GL_FRONT_AND_BACK,
            GL_FILL
        );

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shader);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}