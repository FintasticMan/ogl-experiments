#include <ctype.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "car.h"
#include "exitcodes.h"
#include "logging.h"
#include "shader.h"

#define WIDTH 1024
#define HEIGHT 1024
#define VSYNC GLFW_FALSE
#define NUM_CARS 8
#define PI 3.141592653589793f

static void error_callback(int const errorcode, char const *const description) {
    tlog(5, "GLFW error: %d %s", errorcode, description);
}

static void APIENTRY gl_error_callback(
    GLenum const source,
    GLenum const type,
    GLuint const id,
    GLenum const severity,
    GLint const length,
    GLchar const *const message,
    void const *const userparam
) {
    if (severity != GL_DEBUG_SEVERITY_NOTIFICATION) {
        tlog(5, "GL_ERROR");
        tlog(5, "    source: 0x%" PRIX32, source);
        tlog(5, "    type: 0x%" PRIX32, type);
        tlog(5, "    id: %" PRIu32, id);
        tlog(5, "    severity: 0x%" PRIX32, severity);
        tlog(5, "    length: %" PRId32, length);
        tlog(5, "    length: %s", message);
        tlog(5, "    userparam: 0x%p", userparam);
    }
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

static void framebuffer_size_callback(
    GLFWwindow *const window, //NOLINT(misc-unused-parameters)
    int const width,
    int const height
) {
    glViewport(0, 0, width, height);
}

#pragma GCC diagnostic pop

int main(int const argc, char const *const *const argv) {
    tlog_init(argc < 2 ? 0 : (uint8_t) strtoul(argv[1], NULL, 0), stderr);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
        return EGLFWINITFAIL;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    GLFWwindow *const window = glfwCreateWindow(
        WIDTH,
        HEIGHT,
        "them ais do be leanrin to drove",
        NULL,
        NULL
    );
    if (!window) {
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLuint shader_program;
    create_program(
        "src/shaders/main.vert",
        NULL,
        NULL,
        NULL,
        "src/shaders/main.frag",
        &shader_program
    );

    FILE *const fpt = fopen("track.csv", "r");
    if (!fpt) {
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

    size_t size_vertices = (size_inner + size_outer) * 2 + size_checkpoints;

    float *const vertices = malloc(size_vertices * sizeof(float));
    float *const vert_inner = vertices;
    float *const vert_outer = vert_inner + size_inner * 2;
    float *const vert_check = vert_outer + size_outer * 2;
    float vert_cars[NUM_CARS * CAR_NUM_VERTICES];

    for (size_t i = 0; i < size_inner; i++) {
        fscanf(fpt, "%f", vert_inner + i / 2 * 4 + i % 2);
    }
    for (size_t i = 0; i < size_inner; i++) {
        vert_inner[i * 2 - i % 2 + 2]
            = vert_inner[(i * 2 - i % 2 + 4) % (size_inner * 2)];
    }
    for (size_t i = 0; i < size_outer; i++) {
        fscanf(fpt, "%f", vert_outer + i / 2 * 4 + i % 2);
    }
    for (size_t i = 0; i < size_outer; i++) {
        vert_outer[i * 2 - i % 2 + 2]
            = vert_outer[(i * 2 - i % 2 + 4) % (size_outer * 2)];
    }
    for (size_t i = 0; i < size_checkpoints; i++) {
        fscanf(fpt, "%f", vert_check + i);
    }

    float car_start[3];
    fscanf(fpt, "%f\t%f\t%f", car_start, car_start + 1, car_start + 2);

    struct car cars[NUM_CARS];
    uint64_t generation = 1;
    for (size_t i = 0; i < NUM_CARS; i++) {
        car_init(cars + i, car_start);
    }

    GLuint vaos[2];
    glGenVertexArrays(2, vaos);
    GLuint vbos[2];
    glGenBuffers(2, vbos);

    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(
        GL_ARRAY_BUFFER,
        (size_vertices * sizeof(float)),
        vertices,
        GL_STATIC_DRAW
    );

    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (GLvoid *) 0
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glVertexAttribPointer(
        0,
        2,
        GL_FLOAT,
        GL_FALSE,
        2 * sizeof(float),
        (GLvoid *) 0
    );
    glEnableVertexAttribArray(0);

    glUseProgram(shader_program);

    double begin_time = 0.0;
    double end_time = 0.0;
    double dt;
    uint64_t frame = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        dt = end_time - begin_time;
        begin_time = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        size_t num_dead_cars = 0;
        for (size_t i = 0; i < NUM_CARS; i++) {
            if (!cars[i].alive) {
                num_dead_cars++;
                continue;
            }

            cars[i].rot += (float
                           ) (glfwGetKey(window, GLFW_KEY_A)
                              - glfwGetKey(window, GLFW_KEY_D))
                * (float) dt * 5.0f;
            if (glfwGetKey(window, GLFW_KEY_W)) {
                cars[i].pos[0] += cosf(cars[i].rot) * (float) dt * 0.35f;
                cars[i].pos[1] += sinf(cars[i].rot) * (float) dt * 0.35f;
            }

            car_update_vertices(cars + i);

            car_update_rays(cars + i, vertices, (size_inner + size_outer) * 2);

            car_update_checkpoints(cars + i, vert_check, size_checkpoints);

            car_update_alive(cars + i, vertices, (size_inner + size_outer) * 2);

            if (!cars[i].alive) {
                num_dead_cars++;
                for (size_t j = 0; j < CAR_NUM_VERTICES; j++) {
                    vert_cars[i * CAR_NUM_VERTICES + j] = 0.0f;
                }
                continue;
            }

            for (size_t j = 0; j < CAR_NUM_VERTICES; j++) {
                vert_cars[i * CAR_NUM_VERTICES + j] = cars[i].vertices[j];
            }
        }

        if (num_dead_cars == NUM_CARS) {
            generation++;
            tlog(0, "gen: %" PRIu64, generation);
            for (size_t i = 0; i < NUM_CARS; i++) {
                car_init(cars + i, car_start);
            }
        }

        glBindVertexArray(vaos[0]);
        glDrawArrays(GL_LINES, 0, (GLsizei) (size_vertices / 2));

        glBindVertexArray(vaos[1]);
        glBufferData(
            GL_ARRAY_BUFFER,
            sizeof vert_cars,
            vert_cars,
            GL_DYNAMIC_DRAW
        );
        glDrawArrays(GL_LINES, 0, sizeof vert_cars / sizeof(float) / 2);

        glfwSwapBuffers(window);

        if (frame % 1024 == 0) {
            if (glfwGetKey(window, GLFW_KEY_T)) {
                tlog(1, "time for frame: %f ms", dt * 1000.0);
                tlog(1, "framerate: %f fps", 1.0 / dt);
            }
        }

        if (glfwGetKey(window, GLFW_KEY_Q)
            || glfwGetKey(window, GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        end_time = glfwGetTime();
        frame++;
    }

    free(vertices);
    glfwDestroyWindow(window);
    glfwTerminate();

    return ESUCCESS;
}
