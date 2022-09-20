#ifndef CAR_H
#define CAR_H

#include <math.h>
#include <stdlib.h>

#include <logging.h>

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
    int alive;
    float fov;
    float ray_angles[CAR_NUM_RAYS];
    float ray_ends[CAR_NUM_RAYS * 2];
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
    car->alive = 1;
    car->fov = PI;
    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
        car->ray_angles[i] = CAR_NUM_RAYS == 1 ? 0.0f : -car->fov * 0.5f + (float) i * car->fov / (float) (CAR_NUM_RAYS - 1);
    }
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

    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
        car->vertices[16 + i * 4 + 0] = car->pos[0];
        car->vertices[16 + i * 4 + 1] = car->pos[1];
    }
}

static inline void l_l_intersect(const float *l1, const float *l2, float *t, float *u) {
    float den = (l1[0] - l1[2]) * (l2[1] - l2[3]) - (l1[1] - l1[3]) * (l2[0] - l2[2]);
    *t = ((l1[0] - l2[0]) * (l2[1] - l2[3]) - (l1[1] - l2[1]) * (l2[0] - l2[2])) / den;
    *u = -((l1[0] - l1[2]) * (l1[1] - l2[1]) - (l1[1] - l1[3]) * (l1[0] - l2[0])) / den;
}

static inline void car_is_colliding(struct car *car, const float *track, size_t track_size) {
    float t;
    float u;
    for (size_t i = 0; i < track_size; i += 4) {
        l_l_intersect(car->vertices + 4, track + i, &t, &u);
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            car->alive = 0;
        }
        l_l_intersect(car->vertices + 12, track + i, &t, &u);
        if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
            car->alive = 0;
        }
    }
}

static inline void car_update_checkpoints(struct car *car, const float *checkpoints, size_t checkpoints_size) {
    float t;
    float u;
    l_l_intersect(car->vertices + 4, checkpoints + (car->checkpoints * 4) % checkpoints_size, &t, &u);
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        car->checkpoints++;
    }
    l_l_intersect(car->vertices + 12, checkpoints + (car->checkpoints * 4) % checkpoints_size, &t, &u);
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        car->checkpoints++;
    }
}

static inline void car_update_rays(struct car *car, const float *track, size_t track_size) {
    float t;
    float u;
    float *us = calloc(track_size / 4, sizeof (float));
    float *ts = calloc(track_size / 4, sizeof (float));
    for (size_t i = 0; i < CAR_NUM_RAYS; i++) {
	car->vertices[16 + i * 4 + 2] = cosf(car->ray_angles[i] + car->rot) + car->pos[0];
	car->vertices[16 + i * 4 + 3] = sinf(car->ray_angles[i] + car->rot) + car->pos[1];
        for (size_t j = 0; j < track_size / 4; j++) {
            l_l_intersect(track + j * 4, car->vertices + 16 + i * 4, &t, &u);
	    if (t >= 0.0f &&  t <= 1.0f  && u >= 0.0f) {
	        us[j] = u;
		ts[j] = t;
	    }
	}
	for (size_t j = 0; j < track_size / 4; j++) {
	    u = 0.0f;
	    t = 0.0f;
	    if (us[j] > u) {
	        u = us[j];
		t = ts[j];
	    }
	}
	//car->vertices[16 + i * 4 + 2] = car->pos[0] + t * (car->vertices[16 + i * 4 + 2] - car->pos[0]);
	//car->vertices[16 + i * 4 + 3] = car->pos[1] + t * (car->vertices[16 + i * 4 + 3] - car->pos[1]);
    }
    free(us);
    free(ts);
}

#undef PI

#endif // CAR_H
