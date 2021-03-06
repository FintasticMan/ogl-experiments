#ifndef SHADER_H
#define SHADER_H

#include <glad/gl.h>

void create_shader(const char *path, GLenum type, GLuint *dest);

void create_program_compute(const char *compute_shader_path, GLuint *dest);

void create_program(
    const char *vertex_shader_path,
    const char *tessellation_control_shader_path,
    const char *tessellation_evaluation_shader_path,
    const char *geometry_shader_path,
    const char *fragment_shader_path,
    GLuint *dest
);

#endif
