#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>

void create_shader(char const *path, GLenum type, GLuint *dest);

void create_program_compute(char const *compute_shader_path, GLuint *dest);

void create_program(
    char const *vertex_shader_path,
    char const *tessellation_control_shader_path,
    char const *tessellation_evaluation_shader_path,
    char const *geometry_shader_path,
    char const *fragment_shader_path,
    GLuint *dest
);

#endif
