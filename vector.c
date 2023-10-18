#include "vector.h"
#include <math.h>
#include <stdlib.h>

const vector_t VEC_ZERO = {.x = 0.0, .y = 0.0};

vector_t vec_add(vector_t v1, vector_t v2) {
  vector_t vec_sum = {.x = v1.x + v2.x, .y = v1.y + v2.y};
  return vec_sum;
}

vector_t vec_subtract(vector_t v1, vector_t v2) {
  vector_t vec_dif = {.x = v1.x - v2.x, .y = v1.y - v2.y};
  return vec_dif;
}

vector_t vec_negate(vector_t v) {
  vector_t vec_inv = {.x = (-1) * v.x, .y = (-1) * v.y};
  return vec_inv;
}

vector_t vec_multiply(double scalar, vector_t v) {
  vector_t vec_mult = {.x = scalar * v.x, .y = scalar * v.y};
  return vec_mult;
}

double vec_dot(vector_t v1, vector_t v2) {
  double sum = v1.x * v2.x + v1.y * v2.y;
  return sum;
}

double vec_cross(vector_t v1, vector_t v2) {
  double cross = v1.x * v2.y - v2.x * v1.y;
  return cross;
}

vector_t vec_rotate(vector_t v, double angle) {
  // rotation about the origin
  vector_t vec_rot = {.x = v.x * cos(angle) - v.y * sin(angle),
                      .y = v.x * sin(angle) + v.y * cos(angle)};
  return vec_rot;
}
