#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <cmath>
#include <iostream>

const int WIDTH = 1280;
const int HEIGHT = 720;

void onResize(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

int main() {

    if (!glfwInit()) {
        std::cerr << "GLFW іске қосылмады\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "2-тапсырма",
        nullptr,
        nullptr
    );

    if (!window) {
        std::cerr << "Терезе жасалмады\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, onResize);
    glfwSwapInterval(1);

    if (gladLoadGL(glfwGetProcAddress) == 0) {
        std::cerr << "GLAD жүктелмеді\n";
        glfwTerminate();
        return -1;
    }

    while (!glfwWindowShouldClose(window)) {

        processInput(window);

        float t = (float)glfwGetTime();

        float r = (std::sin(t * 3.0f) + 1.0f) * 0.5f * 0.3f;
        float g = (std::sin(t * 2.0f) + 1.0f) * 0.5f * 0.3f;

        glClearColor(r, g, 0.35f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();

    return 0;
}