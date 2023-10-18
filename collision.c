#include "collision.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// find separation axis
vector_t find_separation_axis(vector_t v1, vector_t v2) {
  vector_t direction = vec_rotate(vec_subtract(v1, v2), M_PI / 2);
  double length = sqrt(pow(v1.x - v2.x, 2) + pow(v1.y - v2.y, 2));
  return vec_multiply(1 / length, direction);
}

// find projection min
double find_projection_min(list_t *shape, vector_t axis) {
  // initialize
  double dot_min = INFINITY;

  // loop through each vertex
  for (size_t i = 0; i < list_size(shape); i++) {
    // find dot product
    vector_t curr_vec = *((vector_t *)list_get(shape, i));
    // compare
    if (vec_dot(curr_vec, axis) < dot_min) {
      dot_min = vec_dot(curr_vec, axis);
    }
  }
  return dot_min;
}

// find projection max
double find_projection_max(list_t *shape, vector_t axis) {
  // initialize
  double dot_max = -INFINITY;
  // loop through each vertex
  for (size_t i = 0; i < list_size(shape); i++) {
    // find dot product
    vector_t curr_vec = *((vector_t *)list_get(shape, i));
    // compare
    if (vec_dot(curr_vec, axis) > dot_max) {
      dot_max = vec_dot(curr_vec, axis);
    }
  }
  return dot_max;
}

// returns the amount of overlap between projections
double get_overlap(double min1, double max1, double min2, double max2) {
  if (min1 > max1) {
    double old1 = min1;
    min1 = max1;
    max1 = old1;
  }
  if (min2 > max2) {
    double old2 = min2;
    min2 = max2;
    max2 = old2;
  }
  // test if proj2 min is within proj1
  if (min1 < min2 && min2 < max1) {
    if (max2 > max1) {
      return fabs(max1 - min2);
    }
    return fabs(max2 - min2);
  }
  // test if proj2 max is within proj1
  if (min1 < max2 && max2 < max1) {
    if (min2 < min1) {
      return fabs(max2 - min1);
    }
    return fabs(max2 - min2);
  }
  // test if proj1 min is within proj2
  if (min2 < min1 && min1 < max2) {
    if (max1 > max2) {
      return fabs(max2 - min1);
    }
    return fabs(max1 - min1);
  }
  // test if proj1 max is within proj2
  if (min2 < max1 && max1 < max2) {
    if (min1 < min2) {
      return fabs(max1 - min2);
    }
    return fabs(max1 - min1);
  }
  // get equals cases
  if (min1 == min2) {
    if (max1 == max2 || max1 < max2) {
      return fabs(max1 - min1);
    } else {
      return fabs(max2 - min2);
    }
  }
  if (min1 == max2 || min2 == max1) {
    return 0.0;
  }
  return -1.0;
}

// returns separation axis where projections have least overlap
vector_t check_projections(list_t *shape1, list_t *shape2) {
  // initialize
  double least_overlap = INFINITY;
  vector_t collision_axis = {.x = 0.0, .y = 0.0};

  // loop through shape1 edges
  for (size_t i = 0; i < list_size(shape1); i++) {
    vector_t v1 = *((vector_t *)list_get(shape1, i % list_size(shape1)));
    vector_t v2 = *((vector_t *)list_get(shape1, (i + 1) % list_size(shape1)));
    vector_t axis = find_separation_axis(v1, v2);

    double min1 = find_projection_min(shape1, axis);
    double max1 = find_projection_max(shape1, axis);
    double min2 = find_projection_min(shape2, axis);
    double max2 = find_projection_max(shape2, axis);
    double curr_overlap = get_overlap(min1, max1, min2, max2);

    // if no overlap, return VEC_ZERO
    if (curr_overlap == INFINITY) {
      return (vector_t){.x = 0.0, .y = 0.0};
    }

    // if current overlap is lowest overlap so far, save overlap & axis
    if (curr_overlap < least_overlap) {
      least_overlap = curr_overlap;
      collision_axis = axis;
    }
  }

  // loop through shape2 edges
  for (size_t i = 0; i < list_size(shape2); i++) {
    vector_t v1 = *((vector_t *)list_get(shape2, i % list_size(shape2)));
    vector_t v2 = *((vector_t *)list_get(shape2, (i + 1) % list_size(shape2)));
    vector_t axis = find_separation_axis(v1, v2);

    double min1 = find_projection_min(shape2, axis);
    double max1 = find_projection_max(shape2, axis);
    double min2 = find_projection_min(shape1, axis);
    double max2 = find_projection_max(shape1, axis);
    double curr_overlap = get_overlap(min1, max1, min2, max2);

    // if no overlap, return VEC_ZERO
    if (curr_overlap == -1.0) {
      return (vector_t){.x = 0.0, .y = 0.0};
    }

    // if current overlap is lowest overlap so far, save overlap & axis
    if (curr_overlap < least_overlap) {
      least_overlap = curr_overlap;
      collision_axis = axis;
    }
  }

  return collision_axis;
}

collision_info_t find_collision(list_t *shape1, list_t *shape2) {
  collision_info_t info;
  vector_t overlap = check_projections(shape1, shape2);
  if (overlap.x == 0.0 || overlap.y == 0.0) {
    info.collided = 0;
  } else {
    info.collided = 1;
  }
  info.axis = overlap;
  return info;
}
