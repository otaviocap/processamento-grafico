#define GLM_ENABLE_EXPERIMENTAL

#include <glad.h>

#include <algorithm>
#include <iostream>
#include <string>

#include "GLFW/glfw3.h"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "glm/gtx/matrix_factorisation.hpp"

class Sprite {
public:
    GLuint textureId = 0;

    float x = 0;
    float y = 0;
    float rotation = glm::radians(180.0f);
    float scaleX = 1.0f;
    float scaleY = 1.0f;
};


class ParallaxLayer : public Sprite {
public:
    float offsetSpeed = 1;
};

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

constexpr int PARALLAX_LAYERS = 6;

constexpr float MAX_CHARACTER_Y = 304.0f;
constexpr float MIN_CHARACTER_Y = 285.0f;
constexpr float MAX_CHARACTER_X = (float) WIDTH;
constexpr float MIN_CHARACTER_X = 0.0f;

Sprite character;

constexpr auto vertexShaderSource = R"GLSL(
 #version 400
 layout (location = 0) in vec3 position;
 layout (location = 1) in vec2 texc;
 out vec2 tex_coord;

 uniform mat4 projection;
 uniform mat4 model;

 void main()
 {
	tex_coord = vec2(texc.s, texc.t);
	gl_Position = projection * model * vec4(position, 1.0);
 }
 )GLSL";

constexpr auto fragmentShaderSource = R"GLSL(
 #version 400
 in vec2 tex_coord;
 out vec4 color;
 uniform sampler2D tex_buff;

uniform vec2 offset;

 void main()
 {
	 color = texture(tex_buff, vec2(tex_coord.x + offset.x, tex_coord.y + offset.y));
 }
 )GLSL";

void framebuffer_size_callback(GLFWwindow *window, const int width, const int height) {
    glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        character.y = std::max(character.y - 1, MIN_CHARACTER_Y);
    } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        character.y = std::min(character.y + 1, MAX_CHARACTER_Y);
    } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        character.x = std::max(character.x - 1, MIN_CHARACTER_X);;
    } else if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        character.x = std::min(character.x + 1, MAX_CHARACTER_X);;
    }
}

GLuint compileShader(const char *shaderSource, int shaderType) {
    const GLuint shaderId = glCreateShader(shaderType);
    glShaderSource(shaderId, 1, &shaderSource, nullptr);
    glCompileShader(shaderId);

    int success;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);

    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                << infoLog << std::endl;
    }

    return shaderId;
}

GLuint createShaderProgram() {
    const GLuint vertexShader =
            compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    const GLuint fragmentShader =
            compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);

    const GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint createVBOAndBind(const GLuint VAO, const float *vertices, const int verticesLength) {
    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, verticesLength * sizeof(float), vertices, GL_STATIC_DRAW);

    return VBO;
}

GLuint setupSprite(float size) {
    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    GLfloat vertices[] = {
        // x     y     z     s     t
        -1.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 1.0f
    };

    for (int i = 0; i < sizeof(vertices) / sizeof(vertices[0]); i++) {
        vertices[i] *= size;
    }

    createVBOAndBind(VAO, vertices, sizeof(vertices) / sizeof(vertices[0]));

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid *) (3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

GLuint loadTexture(std::string filePath) {
    GLuint texID;

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;

    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        if (nrChannels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        } else // png
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}


void drawSprite(const GLuint modelLoc, const GLuint offsetLoc, const Sprite &sprite, float xTexOffset,
                float yTexOffset) {
    glBindTexture(GL_TEXTURE_2D, sprite.textureId);

    auto model = glm::mat4(1);

    model = glm::translate(model, glm::vec3(sprite.x, sprite.y, 0.0));
    model = glm::rotate(model, sprite.rotation, glm::vec3(0, 0, 1));
    model = glm::scale(model, glm::vec3(sprite.scaleX, sprite.scaleY, 1.0f));

    glUniform2f(offsetLoc, xTexOffset, yTexOffset);
    glUniformMatrix4fv(modelLoc, 1,GL_FALSE, value_ptr(model));
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void generateParallaxLayers(ParallaxLayer (&parallaxLayers)[6]) {
    for (int i = 0; i < PARALLAX_LAYERS; i++) {
        ParallaxLayer pl = {};
        pl.x = WIDTH / 2;
        pl.y = HEIGHT / 2;
        pl.scaleX = WIDTH / 1.8f;
        pl.scaleY = HEIGHT / 1.8f;
        pl.offsetSpeed = (float) i / 16.0f;
        pl.textureId = loadTexture("../assets/m4/" + std::to_string(i) + ".png");
        glTexParameteri(pl.textureId, GL_TEXTURE_WRAP_S, GL_REPEAT);

        parallaxLayers[i] = pl;
    }
}

void generateCharacter() {
    character.x = (float) WIDTH / 2;
    character.y = (float) HEIGHT / 2;
    character.scaleX = 25.0f;
    character.scaleY = 25.0f;
    character.rotation = glm::radians(170.0f);
    character.textureId = loadTexture("../assets/m4/character.png");
}

void drawCharacter(GLint offsetLoc, GLint modelLoc, float xModifier, float yModifier) {
    character.x += xModifier;
    character.y += yModifier;
    drawSprite(modelLoc, offsetLoc, character, 1.0f, 0.0f);
    character.x -= xModifier;
    character.y -= yModifier;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_SAMPLES, 8);

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "M4 - Mapeamento de Texturas - Otávio", nullptr, nullptr);

    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    const GLuint shaderProgram = createShaderProgram();
    GLuint VAO = setupSprite(1);

    ParallaxLayer parallaxLayers[6];
    generateParallaxLayers(parallaxLayers);
    generateCharacter();

    glUseProgram(shaderProgram);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(shaderProgram, "tex_buff"), 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_ALWAYS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLint offsetLoc = glGetUniformLocation(shaderProgram, "offset");
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");

    glm::mat4 projection = glm::ortho(0.0f, (float) WIDTH, (float) HEIGHT, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(
        glGetUniformLocation(shaderProgram, "projection"),
        1,
        GL_FALSE,
        value_ptr(projection)
    );

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(VAO);
        double currentTime = glfwGetTime();

        for (int i = 0; i < PARALLAX_LAYERS; i++) {
            ParallaxLayer layer = parallaxLayers[i];

            drawSprite(modelLoc, offsetLoc, layer,
                       (-currentTime + character.x / 1500) * layer.offsetSpeed,
                       (character.y - HEIGHT / 2) / 100 * layer.offsetSpeed
            );
        }

        character.y += 10 * sin(currentTime);
        drawCharacter(offsetLoc, modelLoc, 0, 0);
        drawCharacter(offsetLoc, modelLoc, -50, 25);
        drawCharacter(offsetLoc, modelLoc, -50, -25);
        character.y -= 10 * sin(currentTime);

        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
