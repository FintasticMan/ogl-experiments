#ifndef CAR_H
#define CAR_H

#include <math.h>
#include <stddef.h>

struct car {
    float pos[2];
    float rot;
    float size[2];
    float hyp;
    float angles[4];
    float vertices[16];
    size_t checkpoints;
};

static inline void car_init(struct car *car, float *car_start) {
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
}

static inline void car_update_vertices(struct car *car) {
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
    car->vertices[10] = car->pos[0] + car->hyp * cosf(car->angles[3] + car->rot);
    car->vertices[11] = car->pos[1] + car->hyp * sinf(car->angles[3] + car->rot);
    car->vertices[12] = car->vertices[10];
    car->vertices[13] = car->vertices[11];
    car->vertices[14] = car->vertices[0];
    car->vertices[15] = car->vertices[1];
}

static inline int l_l_intersect(const float *l1, const float *l2) {
    float den = (l1[0] - l1[2]) * (l2[1] - l2[3]) - (l1[1] - l1[3]) * (l2[0] - l2[2]);
    float t = ((l1[0] - l2[0]) * (l2[1] - l2[3]) - (l1[1] - l2[1]) * (l2[0] - l2[2])) / den;
    float u = -((l1[0] - l1[2]) * (l1[1] - l2[1]) - (l1[1] - l1[3]) * (l1[0] - l2[0])) / den;

    return t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f;
}

int car_is_colliding(const struct car *car, const float *track, size_t size_track);
void car_update_checkpoints(struct car *car, const float *checkpoints, size_t size_checkpoints);

#endif
