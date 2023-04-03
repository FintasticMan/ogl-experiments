#ifndef CAR_H
#define CAR_H

#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#define CAR_NUM_RAYS 7
#define CAR_NUM_VERTICES (16 + CAR_NUM_RAYS * 4)

#define PI 3.141592653589793f

struct car {
    float pos[2];
    float rot;
    float size[2];
    float hyp;
    float angles[4];
    float vertices[CAR_NUM_VERTICES];
    size_t checkpoints;
    bool alive;
    float fov;
    float ray_angles[CAR_NUM_RAYS];
    float ray_ends[CAR_NUM_RAYS * 2];
};

static inline void
    car_init(struct car *const car, float const *const car_start) {
    car->pos[0] = car_start[0];
    car->pos[1] = car_start[1];
    car->rot = car_start[2];
    car->size[0] = 0.05f;
    car->size[1] = 0.025f;
    car->hyp = hypotf(car->size[0], car->size[1]) * 0.5f;
    car->angles[0] = atan2f(car->size[1], car->size[0]);
    car->angles[1] = atan2f(-car->size[1], car->size[0]);
    car->angles[2] = atan2f(-car->size[1], -car->size[0]);
    car->angles[3] = atan2f(car->size[1], -car->size[0]);
    car->checkpoints = 0;
    car->alive = 1;
    car->fov = PI;
    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
        car->ray_angles[i] = CAR_NUM_RAYS == 1 ? 0.0f
                                               : -car->fov * 0.5f
                + (float) i * car->fov / (float) (CAR_NUM_RAYS - 1);
    }
}

static inline void car_update_vertices(struct car *const car) {
    car->vertices[0] = car->pos[0] + car->hyp * cosf(car->angles[0] + car->rot);
    car->vertices[1] = car->pos[1] + car->hyp * sinf(car->angles[0] + car->rot);
    car->vertices[2] = car->pos[0] + car->hyp * cosf(car->angles[1] + car->rot);
    car->vertices[3] = car->pos[1] + car->hyp * sinf(car->angles[1] + car->rot);
    car->vertices[4] = car->vertices[2];
    car->vertices[5] = car->vertices[3];
    car->vertices[6] = car->pos[0] + car->hyp * cosf(car->angles[2] + car->rot);
    car->vertices[7] = car->pos[1] + car->hyp * sinf(car->angles[2] + car->rot);
    car->vertices[8] = car->vertices[6];
    car->vertices[9] = car->vertices[7];
    car->vertices[10]
        = car->pos[0] + car->hyp * cosf(car->angles[3] + car->rot);
    car->vertices[11]
        = car->pos[1] + car->hyp * sinf(car->angles[3] + car->rot);
    car->vertices[12] = car->vertices[10];
    car->vertices[13] = car->vertices[11];
    car->vertices[14] = car->vertices[0];
    car->vertices[15] = car->vertices[1];

    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
        car->vertices[16 + i * 4 + 0] = car->pos[0];
        car->vertices[16 + i * 4 + 1] = car->pos[1];
    }
}

// see
// https://en.wikipedia.org/wiki/Line%E2%80%93line_intersection#Given_two_points_on_each_line
static inline void l_l_intersect(
    float const *const l1,
    float const *const l2,
    float *const t,
    float *const u
) {
    float const x1 = l1[0];
    float const y1 = l1[1];
    float const x2 = l1[2];
    float const y2 = l1[3];

    float const x3 = l2[0];
    float const y3 = l2[1];
    float const x4 = l2[2];
    float const y4 = l2[3];

    float const den = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    *t = ((x1 - x3) * (y3 - y4) - (y1 - y3) * (x3 - x4)) / den;
    *u = ((x1 - x3) * (y1 - y2) - (y1 - y3) * (x1 - x2)) / den;
}

static inline void car_update_alive(
    struct car *const car,
    float const *const track,
    size_t const track_size
) {
    for (size_t i = 0; i < track_size; i += 4) {
        float t;
        float u;
        l_l_intersect(track + i, car->vertices + 4, &t, &u);
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            car->alive = false;
            return;
        }
        l_l_intersect(track + i, car->vertices + 12, &t, &u);
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            car->alive = false;
            return;
        }
    }
}

static inline void car_update_checkpoints(
    struct car *const car,
    float const *const checkpoints,
    size_t const checkpoints_size
) {
    float t;
    float u;
    l_l_intersect(
        checkpoints + (car->checkpoints * 4) % checkpoints_size,
        car->vertices + 4,
        &t,
        &u
    );
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        car->checkpoints++;
    }
    l_l_intersect(
        checkpoints + (car->checkpoints * 4) % checkpoints_size,
        car->vertices + 12,
        &t,
        &u
    );
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        car->checkpoints++;
    }
}

static inline void car_update_rays(
    struct car *const car,
    float const *const track,
    size_t const track_size
) {
    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
        car->vertices[16 + i * 4 + 2]
            = car->pos[0] + cosf(car->ray_angles[i] + car->rot);
        car->vertices[16 + i * 4 + 3]
            = car->pos[1] + sinf(car->ray_angles[i] + car->rot);
        float min_u = 0.0f;
        for (size_t j = 0; j < track_size / 4; j++) {
            float t;
            float u;
            l_l_intersect(track + j * 4, car->vertices + 16 + i * 4, &t, &u);
            if (t >= 0.0f && t <= 1.0f && u > 0.0f
                && (u < min_u || min_u == 0.0f)) {
                min_u = u;
            }
        }
        car->vertices[16 + i * 4 + 2] = car->pos[0]
            + min_u * (car->vertices[16 + i * 4 + 2] - car->pos[0]);
        car->vertices[16 + i * 4 + 3] = car->pos[1]
            + min_u * (car->vertices[16 + i * 4 + 3] - car->pos[1]);
    }
}

#undef PI

#endif // CAR_H
