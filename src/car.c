#include <car.h>

int car_is_colliding(const struct car *car, const float *track, size_t track_size) {
    for (size_t i = 0; i < track_size; i += 4) {
        if (l_l_intersect(car->vertices + 4, track + i)) {
            return 1;
        }
        if (l_l_intersect(car->vertices + 12, track + i)) {
            return 1;
        }
    }
    return 0;
}

void car_update_checkpoints(struct car *car, const float *checkpoints, size_t checkpoints_size) {
    if (l_l_intersect(car->vertices + 4, checkpoints + (car->checkpoints * 4) % checkpoints_size) || l_l_intersect(car->vertices + 12, checkpoints + (car->checkpoints * 4) % checkpoints_size)) {
        car->checkpoints++;
    }
}
