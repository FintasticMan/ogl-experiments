#include <stdio.h>
#include <stdlib.h>

#include <exitcodes.h>
#include <logging.h>
#include <shader.h>

void create_shader(const char *path, GLenum type, GLuint *dest) {
    *dest = glCreateShader(type);

    FILE *fp = fopen(path, "rb");
    if (fp == NULL) {
        exit(EFOPENFAIL);
    }

    fseek(fp, 0, SEEK_END);

    size_t file_size = ftell(fp);

    GLchar *source = malloc(file_size + sizeof (GLchar));
    if (source == NULL) {
        exit(EALLOCFAIL);
    }

    fseek(fp, 0, SEEK_SET);
    fread(source, 1, file_size, fp);
    source[file_size] = '\0';

    fclose(fp);

    glShaderSource(*dest, 1, (const GLchar *const *) &source, NULL);

    glCompileShader(*dest);

    GLint success = 0;
    glGetShaderiv(*dest, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint error_len = 0;
        glGetShaderiv(*dest, GL_INFO_LOG_LENGTH, &error_len);
        GLchar error_string[error_len + 1];
        glGetShaderInfoLog(*dest, error_len, &error_len, error_string);

        glDeleteShader(*dest);

        tlog(5, "SHADER COMPILE ERROR: %s\n%s\n", path, error_string);
        exit(EGLSHADERERROR);
    }

    free(source);
}

void create_program_compute(const char *compute_shader_path, GLuint *dest) {
    *dest = glCreateProgram();

    GLuint compute_shader = 0;
    create_shader(compute_shader_path, GL_VERTEX_SHADER, &compute_shader);

    glAttachShader(*dest, compute_shader);

    glLinkProgram(*dest);

    GLint success = 0;
    glGetProgramiv(*dest, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        tlog(5, "SHADER LINK ERROR\n");
        exit(EGLSHADERERROR);
    }

    glDeleteShader(compute_shader);
}

void create_program(
    const char *vertex_shader_path,
    const char *tessellation_control_shader_path,
    const char *tessellation_evaluation_shader_path,
    const char *geometry_shader_path,
    const char *fragment_shader_path,
    GLuint *dest
) {
    *dest = glCreateProgram();

    GLuint vertex_shader = 0;
    GLuint tessellation_control_shader = 0;
    GLuint tessellation_evaluation_shader = 0;
    GLuint geometry_shader = 0;
    GLuint fragment_shader = 0;

    if (vertex_shader_path != NULL) {
        create_shader(vertex_shader_path, GL_VERTEX_SHADER, &vertex_shader);
        glAttachShader(*dest, vertex_shader);
    }
    if (tessellation_control_shader_path != NULL) {
        create_shader(
            tessellation_control_shader_path,
            GL_TESS_CONTROL_SHADER,
            &tessellation_control_shader
        );
        glAttachShader(*dest, tessellation_control_shader);
    }
    if (tessellation_evaluation_shader_path != NULL) {
        create_shader(
            tessellation_evaluation_shader_path,
            GL_TESS_EVALUATION_SHADER,
            &tessellation_evaluation_shader
        );
        glAttachShader(*dest, tessellation_evaluation_shader);
    }
    if (geometry_shader_path != NULL) {
        create_shader(
            geometry_shader_path,
            GL_GEOMETRY_SHADER,
            &geometry_shader
        );
        glAttachShader(*dest, geometry_shader);
    }
    if (fragment_shader_path != NULL) {
        create_shader(
            fragment_shader_path,
            GL_FRAGMENT_SHADER,
            &fragment_shader
        );
        glAttachShader(*dest, fragment_shader);
    }

    glLinkProgram(*dest);

    GLint success = 0;
    glGetProgramiv(*dest, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        tlog(5, "SHADER LINK ERROR\n");
        exit(EGLSHADERERROR);
    }

    if (vertex_shader_path != NULL) {
        glDeleteShader(vertex_shader);
    }
    if (tessellation_control_shader_path != NULL) {
        glDeleteShader(tessellation_control_shader);
    }
    if (tessellation_evaluation_shader_path != NULL) {
        glDeleteShader(tessellation_evaluation_shader);
    }
    if (geometry_shader_path != NULL) {
        glDeleteShader(geometry_shader);
    }
    if (fragment_shader_path != NULL) {
        glDeleteShader(fragment_shader);
    }
}
