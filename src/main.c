#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <exitcodes.h>
#include <logging.h>
#include <shader.h>

#define WIDTH 1024
#define HEIGHT 1024
#define VSYNC GLFW_FALSE
#define NUM_CARS 1
#define PI 3.141592653589793f

struct car {
    float pos[2];
    float rot;
    float size[2];
    float hyp;
    float angles[4];
};

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

int main(int argc, char **argv) {
    tlog_init(argc < 2 ? 0 : (uint8_t) strtoul(argv[1], NULL, 0), stderr);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        return EGLFWINITFAIL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

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

    GLuint shader_program;
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
    size_t size_outer = 0;
    size_t size_checkpoints = 0;
    int c;
    int prev_c = 0;
    while ((c = fgetc(fpt)) != EOF && !(c == '\n' && prev_c == '\n')) {
        size_inner += !isdigit(c) && c != '.' && c != '-';
        prev_c = c;
    }
    prev_c = 0;
    while ((c = fgetc(fpt)) != EOF && !(c == '\n' && prev_c == '\n')) {
        size_outer += !isdigit(c) && c != '.' && c != '-';
        prev_c = c;
    }
    prev_c = 0;
    while ((c = fgetc(fpt)) != EOF && !(c == '\n' && prev_c == '\n')) {
        size_checkpoints += !isdigit(c) && c != '.' && c != '-';
        prev_c = c;
    }
    rewind(fpt);

    size_t size_vertices = size_inner + size_outer + size_checkpoints;
    size_t size_indices = size_inner + size_outer + size_checkpoints / 2;
    tlog(0, "%zu %zu\n", size_vertices, size_indices);

    float *vertices = malloc(size_vertices * sizeof (float));
    float *vert_inner = vertices;
    float *vert_outer = vert_inner + size_inner;
    float *vert_check = vert_outer + size_outer;
    float vert_cars[NUM_CARS * 8];

    GLuint *indices = malloc(size_indices * sizeof (GLuint));
    GLuint *ind_inner = indices;
    GLuint *ind_outer = ind_inner + size_inner;
    GLuint *ind_check = ind_outer + size_outer;
    GLuint ind_cars[NUM_CARS * 8];

    float car_start[3];

    for (size_t i = 0; i < size_inner; i++) {
        fscanf(fpt, "%f", vert_inner + i);
        ind_inner[i] = (i == size_inner - 1) ? 0 : (i + 1) / 2;
        tlog(0, "%f %u\n", (double) vert_inner[i], ind_inner[i]);
    }
    for (size_t i = 0; i < size_outer; i++) {
        fscanf(fpt, "%f", vert_outer + i);
        ind_outer[i] = (i == size_outer - 1) ? size_inner / 2 : (i + size_inner + 1) / 2;
        tlog(0, "%f %u\n", (double) vert_outer[i], ind_outer[i]);
    }
    for (size_t i = 0; i < size_checkpoints; i++) {
        fscanf(fpt, "%f", vert_check + i);
        ind_check[i / 2] = (i + size_inner + size_outer) / 2;
        tlog(0, "%f %u\n", (double) vert_check[i], ind_check[i / 2]);
    }
    fscanf(fpt, "%f\t%f\t%f", car_start, car_start + 1, car_start + 2);
    tlog(0, "%f %f %f\n", (double) car_start[0], (double) car_start[1], (double) car_start[2]);

    struct car cars[NUM_CARS];
    for (size_t i = 0; i < NUM_CARS; i++) {
        cars[i].pos[0] = car_start[0];
        cars[i].pos[1] = car_start[1];
        cars[i].rot = car_start[2];
        cars[i].size[0] = 0.1f;
        cars[i].size[1] = 0.05f;
        cars[i].hyp = hypotf(cars[i].size[0], cars[i].size[1]) * 0.5f;
        cars[i].angles[0] = atan2f(cars[i].size[1], cars[i].size[0]);
        cars[i].angles[1] = atan2f(-cars[i].size[1], cars[i].size[0]);
        cars[i].angles[2] = atan2f(-cars[i].size[1], -cars[i].size[0]);
        cars[i].angles[3] = atan2f(cars[i].size[1], -cars[i].size[0]);
    }

    for (size_t i = 0; i < NUM_CARS; i++) {
        ind_cars[i + 0] = i / 2 + 0;
        ind_cars[i + 1] = i / 2 + 1;
        ind_cars[i + 2] = i / 2 + 1;
        ind_cars[i + 3] = i / 2 + 2;
        ind_cars[i + 4] = i / 2 + 2;
        ind_cars[i + 5] = i / 2 + 3;
        ind_cars[i + 6] = i / 2 + 3;
        ind_cars[i + 7] = i / 2 + 0;

        vert_cars[i + 0] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[0] + cars[i].rot);
        vert_cars[i + 1] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[0] + cars[i].rot);
        vert_cars[i + 2] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[1] + cars[i].rot);
        vert_cars[i + 3] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[1] + cars[i].rot);
        vert_cars[i + 4] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[2] + cars[i].rot);
        vert_cars[i + 5] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[2] + cars[i].rot);
        vert_cars[i + 6] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[3] + cars[i].rot);
        vert_cars[i + 7] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[3] + cars[i].rot);
    }

    GLuint vaos[2];
    glGenVertexArrays(2, vaos);
    GLuint vbos[2];
    glGenBuffers(2, vbos);
    GLuint ebos[2];
    glGenBuffers(2, ebos);

    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, size_vertices * sizeof (float), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_indices * sizeof (GLuint), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    free(vertices);
    free(indices);

    glUseProgram(shader_program);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < NUM_CARS; i++) {
            cars[i].rot += (float) (glfwGetKey(window, GLFW_KEY_A) - glfwGetKey(window, GLFW_KEY_D)) * 0.001f;

            vert_cars[i + 0] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[0] + cars[i].rot);
            vert_cars[i + 1] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[0] + cars[i].rot);
            vert_cars[i + 2] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[1] + cars[i].rot);
            vert_cars[i + 3] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[1] + cars[i].rot);
            vert_cars[i + 4] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[2] + cars[i].rot);
            vert_cars[i + 5] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[2] + cars[i].rot);
            vert_cars[i + 6] = cars[i].pos[0] + cars[i].hyp * cosf(cars[i].angles[3] + cars[i].rot);
            vert_cars[i + 7] = cars[i].pos[1] + cars[i].hyp * sinf(cars[i].angles[3] + cars[i].rot);
        }

        glBindVertexArray(vaos[0]);
        glDrawElements(GL_LINES, size_indices, GL_UNSIGNED_INT, 0);
        glBindVertexArray(vaos[1]);
        glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vert_cars), vert_cars, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebos[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ind_cars), ind_cars, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (GLvoid *) 0);
        glEnableVertexAttribArray(0);
        glDrawElements(GL_LINES, sizeof(ind_cars) / sizeof (GLuint), GL_UNSIGNED_INT, 0);

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
