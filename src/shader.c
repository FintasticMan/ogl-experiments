#include "shader.h"

#include <stdio.h>
#include <stdlib.h>

#include "exitcodes.h"
#include "logging.h"

void create_shader(char const * const path, GLenum const type, GLuint * const dest) {
    *dest = glCreateShader(type);

    FILE * const fpt = fopen(path, "rb");
    if (!fpt) {
        exit(EFOPENFAIL);
    }

    fseek(fpt, 0, SEEK_END);

    size_t const file_size = ftell(fpt);

    GLchar * const source = malloc(file_size + sizeof (GLchar));
    if (!source) {
        exit(EALLOCFAIL);
    }

    fseek(fpt, 0, SEEK_SET);
    fread(source, 1, file_size, fpt);
    source[file_size] = '\0';

    fclose(fpt);

    glShaderSource(*dest, 1, (GLchar const * const *) &source, NULL);

    glCompileShader(*dest);

    GLint success = 0;
    glGetShaderiv(*dest, GL_COMPILE_STATUS, &success);
    if (success != GL_TRUE) {
        GLint error_len = 0;
        glGetShaderiv(*dest, GL_INFO_LOG_LENGTH, &error_len);
        GLchar error_string[error_len + 1];
        glGetShaderInfoLog(*dest, error_len, &error_len, error_string);

        glDeleteShader(*dest);

        tlog(5, "SHADER COMPILE ERROR: %s\n%s", path, error_string);
        exit(EGLSHADERERROR);
    }

    free(source);
}

void create_program_compute(char const * const compute_shader_path, GLuint * const dest) {
    *dest = glCreateProgram();

    GLuint compute_shader = 0;
    create_shader(compute_shader_path, GL_VERTEX_SHADER, &compute_shader);

    glAttachShader(*dest, compute_shader);

    glLinkProgram(*dest);

    GLint success = 0;
    glGetProgramiv(*dest, GL_LINK_STATUS, &success);
    if (success != GL_TRUE) {
        tlog(5, "SHADER LINK ERROR");
        exit(EGLSHADERERROR);
    }

    glDeleteShader(compute_shader);
}

void create_program(
    char const * const vertex_shader_path,
    char const * const tessellation_control_shader_path,
    char const * const tessellation_evaluation_shader_path,
    char const * const geometry_shader_path,
    char const * const fragment_shader_path,
    GLuint * const dest
) {
    *dest = glCreateProgram();

    GLuint vertex_shader = 0;
    GLuint tessellation_control_shader = 0;
    GLuint tessellation_evaluation_shader = 0;
    GLuint geometry_shader = 0;
    GLuint fragment_shader = 0;

    if (vertex_shader_path) {
        create_shader(vertex_shader_path, GL_VERTEX_SHADER, &vertex_shader);
        glAttachShader(*dest, vertex_shader);
    }
    if (tessellation_control_shader_path) {
        create_shader(
            tessellation_control_shader_path,
            GL_TESS_CONTROL_SHADER,
            &tessellation_control_shader
        );
        glAttachShader(*dest, tessellation_control_shader);
    }
    if (tessellation_evaluation_shader_path) {
        create_shader(
            tessellation_evaluation_shader_path,
            GL_TESS_EVALUATION_SHADER,
            &tessellation_evaluation_shader
        );
        glAttachShader(*dest, tessellation_evaluation_shader);
    }
    if (geometry_shader_path) {
        create_shader(
            geometry_shader_path,
            GL_GEOMETRY_SHADER,
            &geometry_shader
        );
        glAttachShader(*dest, geometry_shader);
    }
    if (fragment_shader_path) {
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
        tlog(5, "SHADER LINK ERROR");
        exit(EGLSHADERERROR);
    }

    if (vertex_shader_path) {
        glDeleteShader(vertex_shader);
    }
    if (tessellation_control_shader_path) {
        glDeleteShader(tessellation_control_shader);
    }
    if (tessellation_evaluation_shader_path) {
        glDeleteShader(tessellation_evaluation_shader);
    }
    if (geometry_shader_path) {
        glDeleteShader(geometry_shader);
    }
    if (fragment_shader_path) {
        glDeleteShader(fragment_shader);
    }
}
