#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/gl.h>

#include <car.h>
#include <exitcodes.h>
#include <logging.h>
#include <shader.h>

#define WIDTH 1024
#define HEIGHT 1024
#define VSYNC GLFW_FALSE
#define NUM_CARS 1
#define PI 3.141592653589793f

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

    size_t size_vertices = (size_inner + size_outer) * 2 + size_checkpoints;

    float *vertices = malloc(size_vertices * sizeof (float));
    float *vert_inner = vertices;
    float *vert_outer = vert_inner + size_inner * 2;
    float *vert_check = vert_outer + size_outer * 2;
    float vert_cars[NUM_CARS * 16];

    for (size_t i = 0; i < size_inner; i++) {
        fscanf(fpt, "%f", vert_inner + i / 2 * 4 + i % 2);
    }
    for (size_t i = 0; i < size_inner; i++) {
        vert_inner[i / 2 * 4 + i % 2 + 2] = vert_inner[(i / 2 * 4 + i % 2 + 4) % (size_inner * 2)];
    }
    for (size_t i = 0; i < size_outer; i++) {
        fscanf(fpt, "%f", vert_outer + i / 2 * 4 + i % 2);
    }
    for (size_t i = 0; i < size_outer; i++) {
        vert_outer[i / 2 * 4 + i % 2 + 2] = vert_outer[(i / 2 * 4 + i % 2 + 4) % (size_outer * 2)];
    }
    for (size_t i = 0; i < size_checkpoints; i++) {
        fscanf(fpt, "%f", vert_check + i);
    }

    float car_start[3];
    fscanf(fpt, "%f\t%f\t%f", car_start, car_start + 1, car_start + 2);

    struct car cars[NUM_CARS];
    for (size_t i = 0; i < NUM_CARS; i++) {
        car_init(cars + i, car_start);
    }

    GLuint vaos[2];
    glGenVertexArrays(2, vaos);
    GLuint vbos[2];
    glGenBuffers(2, vbos);

    glBindVertexArray(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[0]);
    glBufferData(GL_ARRAY_BUFFER, size_vertices * sizeof (float), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vbos[1]);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof (float), (GLvoid *) 0);
    glEnableVertexAttribArray(0);

    glUseProgram(shader_program);

    double begin_time = 0.0;
    double end_time = 0.0;
    double dt;
    unsigned long long frame = 0;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        dt = end_time - begin_time;
        begin_time = glfwGetTime();

        glClear(GL_COLOR_BUFFER_BIT);

        for (size_t i = 0; i < NUM_CARS; i++) {
            cars[i].rot += (float) (glfwGetKey(window, GLFW_KEY_A) - glfwGetKey(window, GLFW_KEY_D)) * (float) dt * 5.0f;
            if (glfwGetKey(window, GLFW_KEY_W)) {
                cars[i].pos[0] += cosf(cars[i].rot) * (float) dt * 0.35f;
                cars[i].pos[1] += sinf(cars[i].rot) * (float) dt * 0.35f;
            }

            car_update_vertices(cars + i);

            for (size_t j = 0; j < 16; j++) {
                vert_cars[i * 16 + j] = cars[i].vertices[j];
            }

            if (car_is_colliding(cars + i, vertices, (size_inner + size_outer) * 2)) {
                //tlog(1, "car %zu is colliding with the track\n", i);
            }

            car_update_checkpoints(cars + i, vert_check, size_checkpoints);
            if (frame % 1024 == 0) {
                tlog(0, "checkpoints %zu\n", cars[i].checkpoints);
            }
        }

        glBindVertexArray(vaos[0]);
        glDrawArrays(GL_LINES, 0, size_vertices);

        glBindVertexArray(vaos[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vert_cars), vert_cars, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_LINES, 0, sizeof(vert_cars) / sizeof (float));

        glfwSwapBuffers(window);

        if (frame % 1024 == 0) {
            if (glfwGetKey(window, GLFW_KEY_T)) {
                tlog(1, "time for frame: %f ms\n", dt * 1000.0);
                tlog(1, "framerate: %f fps\n", 1.0 / dt);
            }
        }

        if (
            glfwGetKey(window, GLFW_KEY_Q) ||
            glfwGetKey(window, GLFW_KEY_ESCAPE)
        ) {
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
