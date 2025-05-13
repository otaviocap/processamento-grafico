
#include <glad.h>
#include <iostream>
#include <vector>

#include "GLFW/glfw3.h"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

constexpr auto vertexShaderSource =
R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;

void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)GLSL";

constexpr auto fragmentShaderSource = R"GLSL(
#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
)GLSL";

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

GLuint compileShader(const char* shaderSource, int shaderType) {
    const GLuint shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &shaderSource, nullptr);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    return shaderId;
}

GLuint createShaderProgram() {
    const GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    const GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    const GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createVBOAndBind(const GLuint VAO, const float* vertices, const int verticesLength) {
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesLength * sizeof(float), vertices, GL_STATIC_DRAW);

    return VBO;
}

GLuint createTriangle(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const float x3,
    const float y3) {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    float vertices[] = {
        x1, y1, 0.0f,
        x2, y2, 0.0f,
        x3, y3, 0.0f,
    };

    createVBOAndBind(VAO, vertices, sizeof(vertices) / sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Otavio Triangulos", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    const GLuint shaderProgram = createShaderProgram();
    std::vector<GLuint> VAOs;
    VAOs.push_back(createTriangle(-0.5f,  -0.5f, 0.5f, -0.5f, 0.0, 0.5f));
    VAOs.push_back(createTriangle(-0.4f,  -0.4f, 0.4f, -0.4f, 0.0, 0.4f));
    VAOs.push_back(createTriangle(-0.3f,  -0.3f, 0.3f, -0.3f, 0.0, 0.3f));
    VAOs.push_back(createTriangle(-0.2f,  -0.2f, 0.2f, -0.2f, 0.0, 0.2f));
    VAOs.push_back(createTriangle(-0.1f,  -0.1f, 0.1f, -0.1f, 0.0, 0.1f));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glUseProgram(shaderProgram);

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (const unsigned int VAO : VAOs) {
            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, 0, 3);
            glBindVertexArray(0);
        }

        glfwSwapBuffers(window);
    }

    for (const unsigned int VAO : VAOs) {
        glDeleteVertexArrays(1, &VAO);
    }

    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}