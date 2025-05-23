#define GLM_ENABLE_EXPERIMENTAL

#include <algorithm>
#include <glad.h>
#include <iomanip>
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
constexpr int HEIGHT = 800;
constexpr int COLUMNS = 10;
constexpr int ROWS = 10;
constexpr int QUAD_WIDTH = WIDTH / COLUMNS;
constexpr int QUAD_HEIGHT = HEIGHT / ROWS;

constexpr double MAX_DISTANCE = sqrt(3.0);
constexpr double TOLERANCE = 0.2;

constexpr int CHAIN_MULTIPLIER = 2;
constexpr int PLAY_COST = 5;

int score = 0;

struct Quad
{
    glm::vec2 position;
    glm::vec3 color;
    bool visible = true;
};

float randomFloat() {
    return static_cast <float> (rand()) / static_cast <float> (RAND_MAX);;
}

glm::vec3 randomColor()
{
    return {randomFloat(), randomFloat(), randomFloat()};
};

constexpr glm::vec3 clearColor = glm::vec3(0.0f, 0.0f, 0.0f);


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

void framebufferSizeCallback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
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

GLuint createQuad(
    const float x1,
    const float y1,
    const float x2,
    const float y2,
    const float x3,
    const float y3,
    const float x4,
    const float y4) {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    float vertices[] = {
        x1, y1, 0.0f,
        x2, y2, 0.0f,
        x3, y3, 0.0f,
        x4, y4, 0.0f,
    };

    createVBOAndBind(VAO, vertices, sizeof(vertices) / sizeof(float));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

Quad quads[COLUMNS][ROWS];
Quad* selectedQuad;

void generateBoard() {
    for (int x = 0; x < COLUMNS; x++) {
        for (int y = 0; y < ROWS; y++) {
            Quad quad = {};
            quad.position = glm::vec2((QUAD_WIDTH / 2) + x * QUAD_WIDTH, (QUAD_HEIGHT / 2) + y * QUAD_HEIGHT);
            quad.color = randomColor();
            quads[x][y] = quad;
        }
    }
}

void checkForQuadEliminationAndAddScore() {
    if (!selectedQuad->visible) {
        selectedQuad = nullptr;
        return;
    }

    selectedQuad->visible = false;

    score -= PLAY_COST;
    int chain = 0;

    for (int x = 0; x < COLUMNS; x++) {
        for (int y = 0; y < ROWS; y++) {
            Quad* currentQuad = &quads[x][y];

            const double distance = sqrt(
                pow( selectedQuad->color.r - currentQuad->color.r,2)
                + pow( selectedQuad->color.g - currentQuad->color.g,2)
                + pow( selectedQuad->color.b - currentQuad->color.b,2));

            double relativeDistance = distance / MAX_DISTANCE;

            if (relativeDistance <= TOLERANCE) {
                currentQuad->visible = false;
                chain++;
            }
        }
    }
    selectedQuad = nullptr;

    score += chain * CHAIN_MULTIPLIER;
}

bool gameHasEnded() {
    for (int x = 0; x < COLUMNS; x++) {
        for (int y = 0; y < ROWS; y++) {
            if (quads[x][y].visible) {
                return false;
            }
        }
    }
    return true;
}

void printScore() {
    std::cout << "Parabéns! você obteve " << score << " pontos" << std::endl;
    std::cout << "Recomeçando" << std::endl;
}

void restartGame() {
    generateBoard();
    score = 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (selectedQuad != nullptr) {
        checkForQuadEliminationAndAddScore();
        if (gameHasEnded()) {
            printScore();
            restartGame();
        };
    }
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Jogo das Cores - Otavio", nullptr, nullptr);

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
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    glfwSetMouseButtonCallback(window,
    [] (GLFWwindow* window, int button, int action, int mods)
        {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
            {
                double xpos, ypos;
                glfwGetCursorPos(window, &xpos, &ypos);

                int x = xpos / QUAD_WIDTH;
                int y = ypos / QUAD_HEIGHT;

                selectedQuad = &quads[x][y];
            }
        }
    );

    const GLuint shaderProgram = createShaderProgram();
    GLuint baseQuadVAO = createQuad(
        -0.5f, 0.5f,
        -0.5f, -0.5f,
        0.5, 0.5f,
        0.5f, -0.5f
    );

    generateBoard();

    glUseProgram(shaderProgram);

    GLint colorLoc = glGetUniformLocation(shaderProgram, "inputColor");
    glm::mat4 projection = glm::ortho(0.0, (double) WIDTH, (double) HEIGHT, 0.0, -1.0, 1.0);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1,
        GL_FALSE,
        value_ptr(projection)
    );

    while(!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        processInput(window);

        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        glBindVertexArray(baseQuadVAO);
        for (int x = 0; x < COLUMNS; x++) {
            for (int y = 0; y < ROWS; y++) {
                Quad quad = quads[x][y];

                glm::mat4 model = glm::mat4(1);

                model = glm::translate(model, glm::vec3(quad.position,  0.0));
                model = glm::scale(model, glm::vec3(QUAD_WIDTH, QUAD_HEIGHT, 1.0));

                glUniformMatrix4fv(
                    glGetUniformLocation(shaderProgram, "model"),
                    1,
                    GL_FALSE,
                    value_ptr(model)
                );

                auto color = quad.visible ? quad.color : clearColor;

                glUniform4f(colorLoc, color.r, color.g, color.b, 1.0f);
                glDrawArrays(GL_TRIANGLE_STRIP, 0, 6);

            }
        }

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &baseQuadVAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}