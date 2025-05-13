#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <glad.h>
#include <iostream>
#include <vector>

#include "GLFW/glfw3.h"
#include "glm/fwd.hpp"
#include "glm/vec2.hpp"
#include "glm/vec3.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/transform.hpp"

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

struct Triangle
{
public:
    glm::vec2 position;
    glm::vec3 color;
};

constexpr glm::vec3 colors[] = {
    glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 1.0f, 1.0f),
    glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(1.0f, 1.0f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(0.5f, 1.0f, 0.5f),
    glm::vec3(0.5f, 1.0f, 1.0f)
};

glm::vec3 randomColor()
{
    return colors[rand() % sizeof(colors) / sizeof(colors[0])];
};


constexpr auto vertexShaderSource =
R"GLSL(
#version 330 core
layout (location = 0) in vec3 aPos;
uniform mat4 projection;
uniform mat4 model;

void main()
{
    gl_Position = projection * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
)GLSL";

constexpr auto fragmentShaderSource = R"GLSL(
#version 330 core
uniform vec4 inputColor;
out vec4 FragColor;

void main()
{
    FragColor = inputColor;
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
    std::vector<Triangle> triangles;

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

    glfwSetWindowUserPointer(window, &triangles);
    glfwSetMouseButtonCallback(window,
    [] (GLFWwindow* window, int button, int action, int mods)
        {

            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                auto triangles = static_cast<std::vector<Triangle>*>(glfwGetWindowUserPointer(window));
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                Triangle triangle = {};
                triangle.position = glm::vec2(xpos,ypos);
                triangle.color = randomColor();

                triangles->push_back(triangle);
            }
        }
    );

    const GLuint shaderProgram = createShaderProgram();
    GLuint triangleVAO = createTriangle(-0.5f,  -0.5f, 0.5f, -0.5f, 0.0, 0.5f);

    Triangle baseTriangle = {};
    baseTriangle.position = glm::vec3(400.0f, 300.0f, 0.0f);
    baseTriangle.color = randomColor();
    triangles.push_back(baseTriangle);

    glUseProgram(shaderProgram);

    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    glm::mat4 projection = glm::ortho(0.0, 800.0, 600.0, 0.0, -1.0, 1.0);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1,
        GL_FALSE,
        value_ptr(projection)
    );

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);


        glBindVertexArray(triangleVAO);
        for (const Triangle triangle : triangles) {
            glm::mat4 model = glm::mat4(1);

            model = glm::translate(model, glm::vec3(triangle.position,  0.0));
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0,0,1));
            model = glm::scale(model, glm::vec3(100, 100, 1.0));

            glUniformMatrix4fv(
                glGetUniformLocation(shaderProgram, "model"),
                1,
                GL_FALSE,
                value_ptr(model)
            );

            glUniform4f(colorLoc, triangle.color.r, triangle.color.g, triangle.color.b, 1.0f);

            glDrawArrays(GL_TRIANGLES, 0, 3);
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &triangleVAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}