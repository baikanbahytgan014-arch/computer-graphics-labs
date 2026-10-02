#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cmath>

struct Point
{
    float x;
    float y;
};

int pyramidHeight = 6;

const int MIN_HEIGHT = 1;
const int MAX_HEIGHT = 20;

Point getPoint(int row, int index, int height)
{
    float side = 1.8f / height;
    float h = side * std::sqrt(3.0f) / 2.0f;

    float x = -row * side / 2.0f + index * side;
    float y = 0.85f - row * h;

    return {x, y};
}

void addVertex(std::vector<float>& data, Point p)
{
    data.push_back(p.x);
    data.push_back(p.y);
    data.push_back(0.0f);
}

std::vector<float> createFilledTriangles(int height)
{
    std::vector<float> data;

    for (int row = 0; row < height; row++)
    {
        for (int i = 0; i <= row; i++)
        {
            int triangleIndex = i * 2;

            Point top = getPoint(row, i, height);
            Point left = getPoint(row + 1, i, height);
            Point right = getPoint(row + 1, i + 1, height);

            if (triangleIndex % 2 == 0)
            {
                addVertex(data, top);
                addVertex(data, left);
                addVertex(data, right);
            }

            if (i < row)
            {
                int invertedIndex = i * 2 + 1;

                Point leftTop = getPoint(row, i, height);
                Point rightTop = getPoint(row, i + 1, height);
                Point bottom = getPoint(row + 1, i + 1, height);

                if (invertedIndex % 2 == 0)
                {
                    addVertex(data, leftTop);
                    addVertex(data, rightTop);
                    addVertex(data, bottom);
                }
            }
        }
    }

    return data;
}

std::vector<float> createGrid(int height)
{
    std::vector<float> data;

    for (int row = 0; row < height; row++)
    {
        for (int i = 0; i <= row; i++)
        {
            Point top = getPoint(row, i, height);
            Point left = getPoint(row + 1, i, height);
            Point right = getPoint(row + 1, i + 1, height);

            addVertex(data, top);
            addVertex(data, left);

            addVertex(data, top);
            addVertex(data, right);

            addVertex(data, left);
            addVertex(data, right);
        }
    }

    return data;
}

const char* vertexShaderSource = R"(
#version 330 core

layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos, 1.0);
}
)";

const char* fragmentShaderSource = R"(
#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.0, 0.3, 1.0, 1.0);
}
)";

void key_callback(
    GLFWwindow* window,
    int key,
    int scancode,
    int action,
    int mods
)
{
    if (action != GLFW_PRESS)
        return;

    if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD)
    {
        if (pyramidHeight < MAX_HEIGHT)
        {
            pyramidHeight++;

            std::cout
                << "Высота: "
                << pyramidHeight
                << std::endl;
        }
    }

    if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT)
    {
        if (pyramidHeight > MIN_HEIGHT)
        {
            pyramidHeight--;

            std::cout
                << "Высота: "
                << pyramidHeight
                << std::endl;
        }
    }

    if (key == GLFW_KEY_R)
    {
        pyramidHeight = 6;

        std::cout
            << "Высота: 6"
            << std::endl;
    }

    if (key == GLFW_KEY_ESCAPE)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    if (!glfwInit())
    {
        std::cout << "Ошибка GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MAJOR,
        3
    );

    glfwWindowHint(
        GLFW_CONTEXT_VERSION_MINOR,
        3
    );

    glfwWindowHint(
        GLFW_OPENGL_PROFILE,
        GLFW_OPENGL_CORE_PROFILE
    );

    GLFWwindow* window = glfwCreateWindow(
        900,
        800,
        "OpenGL Pyramid",
        nullptr,
        nullptr
    );

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    int version = gladLoadGL(glfwGetProcAddress);

    if (version == 0)
    {
        std::cout << "Ошибка GLAD!" << std::endl;

        glfwDestroyWindow(window);
        glfwTerminate();

        return -1;
    }

    glfwSetKeyCallback(
        window,
        key_callback
    );

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        nullptr
    );

    glCompileShader(vertexShader);

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        nullptr
    );

    glCompileShader(fragmentShader);

    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(
        shaderProgram,
        vertexShader
    );

    glAttachShader(
        shaderProgram,
        fragmentShader
    );

    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    unsigned int filledVAO;
    unsigned int filledVBO;

    unsigned int gridVAO;
    unsigned int gridVBO;

    glGenVertexArrays(
        1,
        &filledVAO
    );

    glGenBuffers(
        1,
        &filledVBO
    );

    glGenVertexArrays(
        1,
        &gridVAO
    );

    glGenBuffers(
        1,
        &gridVBO
    );

    int oldHeight = 0;

    std::vector<float> filled;
    std::vector<float> grid;

    while (!glfwWindowShouldClose(window))
    {
        if (oldHeight != pyramidHeight)
        {
            filled =
                createFilledTriangles(
                    pyramidHeight
                );

            grid =
                createGrid(
                    pyramidHeight
                );

            oldHeight =
                pyramidHeight;

            glBindVertexArray(
                filledVAO
            );

            glBindBuffer(
                GL_ARRAY_BUFFER,
                filledVBO
            );

            glBufferData(
                GL_ARRAY_BUFFER,
                filled.size() * sizeof(float),
                filled.data(),
                GL_DYNAMIC_DRAW
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

            glBindVertexArray(
                gridVAO
            );

            glBindBuffer(
                GL_ARRAY_BUFFER,
                gridVBO
            );

            glBufferData(
                GL_ARRAY_BUFFER,
                grid.size() * sizeof(float),
                grid.data(),
                GL_DYNAMIC_DRAW
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
        }

        glClearColor(
            0.05f,
            0.05f,
            0.05f,
            1.0f
        );

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(
            filledVAO
        );

        glDrawArrays(
            GL_TRIANGLES,
            0,
            filled.size() / 3
        );

        glBindVertexArray(
            gridVAO
        );

        glDrawArrays(
            GL_LINES,
            0,
            grid.size() / 3
        );

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    glDeleteVertexArrays(
        1,
        &filledVAO
    );

    glDeleteBuffers(
        1,
        &filledVBO
    );

    glDeleteVertexArrays(
        1,
        &gridVAO
    );

    glDeleteBuffers(
        1,
        &gridVBO
    );

    glDeleteProgram(
        shaderProgram
    );

    glfwDestroyWindow(window);

    glfwTerminate();

    return 0;
}