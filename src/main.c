#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <exitcodes.h>
#include <shader.h>
#include <logging.h>

#define WIDTH 1920
#define HEIGHT 1080
#define VSYNC GLFW_TRUE

static void error_callback(int errorcode, const char *description) {
    tlog(5, "GLFW error: %d %s\n", errorcode, description);
}

static void APIENTRY gl_error_callback(
    GLenum source,
    GLenum type,
    GLuint id,
    GLenum severity,
    GLint length,
    const GLchar *message,
    const void *userparam
) {
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        tlog(
            5,
            "GL_ERROR\nsource: 0x%x\ntype: 0x%x\nid: %u\nseverity: 0x%x\n"
            "length: %d\nmessage: %s\nuserparam: 0x%p\n",
            source,
            type,
            id,
            severity,
            length,
            message,
            userparam
        );
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static void framebuffer_size_callback(
    GLFWwindow *window, //NOLINT(misc-unused-parameters)
    int width,
    int height
) {
    glViewport(0, 0, width, height);
}
#pragma GCC diagnostic pop

int main() {
    tlog_init(0, stderr);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        return EGLFWINITFAIL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "them ais do be leanrin to drove",
        NULL,
        NULL
    );
    if (window == NULL) {
        glfwTerminate();
        return EGLFWWINDOWFAIL;
    }
    glfwMakeContextCurrent(window);

    glfwSwapInterval(VSYNC);

    if (!gladLoadGL(glfwGetProcAddress)) {
        glfwDestroyWindow(window);
        glfwTerminate();
        return ELOADGLFAIL;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(gl_error_callback, NULL);

    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    GLuint shader_program = 0;
    create_program(
        "src/shaders/main.vert",
        NULL,
        NULL,
        NULL,
        "src/shaders/main.frag",
        &shader_program
    );

    GLfloat vertices[] = {
        -0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f,
        0.5f, -0.5f
    };

    GLuint vao = 0;
    GLuint vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shader_program);
        glBindVertexArray(vao);
        glDrawArrays(GL_LINES, 0, 2);

        glfwSwapBuffers(window);

        if (
            glfwGetKey(window, GLFW_KEY_Q) ||
            glfwGetKey(window, GLFW_KEY_ESCAPE)
        ) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return ESUCCESS;
}
