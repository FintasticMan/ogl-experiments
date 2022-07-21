#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <exitcodes.h>
#include <logging.h>
#include <shader.h>

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

    FILE *fpt = fopen("track.csv", "r");
    if (fpt == NULL) {
        return EFOPENFAIL;
    }

    size_t size_inner = 0;
    int c = 0;
    int prev_c = 0;
    while ((c = fgetc(fpt)) != EOF && !(c == '\n' && prev_c == '\n')) {
        size_inner += !isdigit(c);
        prev_c = c;
    }
    rewind(fpt);
    tlog(0, "%zu\n", size_inner);

    float *vertices = malloc(size_inner * sizeof (float));
    GLuint *indices = malloc(size_inner * sizeof (GLuint));

    for (size_t i = 0; i < size_inner; i++) {
        fscanf(fpt, "%f", vertices + i);
        vertices[i] = vertices[i] / ((i % 2) ? 720.f : 1280.f) * 2.f - 1.f;
        if (i % 2) {
            vertices[i] = -vertices[i];
        }
        indices[i] = (i == size_inner - 1) ? 0 : (i + 1) / 2;
        tlog(0, "%f %d\n", (double) vertices[i], indices[i]);
    }

    GLuint vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint vbo = 0;
    glGenBuffers(1, &vbo);

    GLuint ebo = 0;
    glGenBuffers(1, &ebo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, size_inner * sizeof (float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_inner * sizeof (GLuint), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    free(vertices);
    free(indices);

    glUseProgram(shader_program);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        //glUseProgram(shader_program);
        //glBindVertexArray(vao);
        glDrawElements(GL_LINES, size_inner, GL_UNSIGNED_INT, 0);

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
