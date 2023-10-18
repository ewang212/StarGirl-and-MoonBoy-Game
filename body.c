#include "color.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct body {
  list_t *vertices;
  void *info;
  rgb_color_t color;
  double mass;
  vector_t velocity;
  vector_t position;
  double angle;
  vector_t force;
  vector_t impulse;
  vector_t acceleration;
  bool flag;
  bool grav;
  bool prev;
  free_func_t info_freer;
} body_t;

body_t *body_init(list_t *shape, double mass, rgb_color_t color) {
  body_t *new_body = malloc(sizeof(body_t));
  assert(new_body != NULL);

  new_body->vertices = shape;
  new_body->color = color;
  new_body->mass = mass;
  vector_t init_velocity = VEC_ZERO;
  new_body->velocity = init_velocity;
  new_body->position = polygon_centroid(shape);
  vector_t init_force = VEC_ZERO;
  new_body->force = init_force;
  vector_t init_impulse = VEC_ZERO;
  new_body->impulse = init_impulse;
  vector_t init_acceleration = VEC_ZERO;
  new_body->acceleration = init_acceleration;
  new_body->info_freer = NULL;
  new_body->angle = 0.0;
  new_body->info = NULL;
  new_body->flag = 0;
  new_body->grav = 0;
  new_body->prev = 0;

  return new_body;
}

body_t *body_init_with_prev_grav_info(bool prev, bool grav_remover,
                                      list_t *shape, double mass,
                                      rgb_color_t color) {
  body_t *new_body = malloc(sizeof(body_t));
  assert(new_body != NULL);

  new_body->vertices = shape;
  new_body->color = color;
  new_body->mass = mass;
  vector_t init_velocity = VEC_ZERO;
  new_body->velocity = init_velocity;
  new_body->position = polygon_centroid(shape);
  vector_t init_force = VEC_ZERO;
  new_body->force = init_force;
  vector_t init_impulse = VEC_ZERO;
  new_body->impulse = init_impulse;
  vector_t init_acceleration = VEC_ZERO;
  new_body->acceleration = init_acceleration;
  new_body->angle = 0.0;
  new_body->flag = 0;
  new_body->prev = prev;
  new_body->grav = grav_remover;
  return new_body;
}

body_t *body_init_with_info(list_t *shape, double mass, rgb_color_t color,
                            void *info, free_func_t info_freer) {
  body_t *new_body = malloc(sizeof(body_t));
  assert(new_body != NULL);

  new_body->vertices = shape;
  new_body->color = color;
  new_body->mass = mass;
  vector_t init_velocity = VEC_ZERO;
  new_body->velocity = init_velocity;
  new_body->position = polygon_centroid(shape);
  vector_t init_force = VEC_ZERO;
  new_body->force = init_force;
  vector_t init_impulse = VEC_ZERO;
  new_body->impulse = init_impulse;
  vector_t init_acceleration = VEC_ZERO;
  new_body->acceleration = init_acceleration;
  new_body->info = info;
  new_body->info_freer = info_freer;
  new_body->angle = 0.0;
  new_body->flag = 0;
  new_body->grav = 0;
  new_body->prev = 0;

  return new_body;
}

void body_free(body_t *body) {
  list_free(body->vertices);
  if (body->info_freer != NULL) {
    body->info_freer(body->info);
  }
  free(body);
}

list_t *body_get_shape(body_t *body) {
  // makes a deep copy
  list_t *new_vertices = list_init(list_size(body->vertices), free);
  for (size_t i = 0; i < list_size(body->vertices); i++) {
    vector_t *vertex = malloc(sizeof(vector_t));
    vector_t *curr = list_get(body->vertices, i);
    vertex->x = curr->x;
    vertex->y = curr->y;
    list_add(new_vertices, vertex);
  }
  return new_vertices;
}

list_t *body_get_vertices(body_t *body) { return body->vertices; }

vector_t body_get_centroid(body_t *body) { return body->position; }

double body_get_mass(body_t *body) { return body->mass; }

vector_t body_get_velocity(body_t *body) { return body->velocity; }

rgb_color_t body_get_color(body_t *body) { return body->color; }

void *body_get_info(body_t *body) { return body->info; }

void body_set_info(body_t *body, void *info) { body->info = info; }

// size_t body_get_force_removal(body_t *body) { return body->remove; }

// void body_set_force_removal(body_t *body, size_t remove) { body->remove =
// remove; }

void body_set_color(body_t *body, rgb_color_t color) { body->color = color; }

bool body_get_grav(body_t *body) { return body->grav; }

void body_set_grav(body_t *body, bool grav) { body->grav = grav; }

// moves centroid & all vertices accordingly
void body_set_centroid(body_t *body, vector_t x) {
  vector_t dist = vec_subtract(x, body->position);
  polygon_translate(body->vertices, dist);

  body->position = x;
}

void body_set_velocity(body_t *body, vector_t v) { body->velocity = v; }

void body_set_fake_rotation(body_t *body, double angle) {
  polygon_rotate(body->vertices, 2 * M_PI - body->angle, body->position);
  polygon_rotate(body->vertices, angle, body->position);
}

void body_set_rotation(body_t *body, double angle) {
  polygon_rotate(body->vertices, angle, body->position);
}

void body_add_force(body_t *body, vector_t force) {
  vector_t old_force = body->force;
  body->force = vec_add(old_force, force);
}

vector_t body_get_force(body_t *body) { return body->force; }

bool body_get_prev_collision(body_t *body) { return body->prev; }

void body_set_prev_collision(body_t *body, bool prev) { body->prev = prev; }

void body_remove_force(body_t *body, vector_t force) {
  vector_t old_force = body->force;
  body->force = vec_subtract(old_force, force);
}

void body_add_impulse(body_t *body, vector_t impulse) {
  body->impulse = vec_add(body->impulse, impulse);
}

void body_set_angle(body_t *body, double angle) { body->angle = angle; }

double body_get_angle(body_t *body) { return body->angle; }

void body_reset_force(body_t *body) {
  body->force = (vector_t){.x = 0, .y = 0};
}

void body_tick(body_t *body, double dt) {
  // finds acceleration then velocity and dist and sets new velocity
  body->acceleration = vec_multiply(1.0 / (body->mass), body->force);
  vector_t old_vel = body_get_velocity(body);
  vector_t older_vel = vec_add(old_vel, vec_multiply(dt, body->acceleration));
  vector_t new_vel =
      vec_add(older_vel, vec_multiply(1.0 / body->mass, body->impulse));
  body_set_velocity(body, new_vel);

  vector_t avg_velocity = vec_multiply(0.5, vec_add(old_vel, new_vel));
  vector_t dist = vec_multiply(dt, avg_velocity);

  // translates vertices and position
  polygon_translate(body->vertices, dist);
  vector_t old_position = body->position;
  body->position = vec_add(old_position, dist);
  body->force = (vector_t){.x = 0.0, .y = 0.0};
  body->impulse = (vector_t){.x = 0.0, .y = 0.0};
}

void body_remove(body_t *body) { body->flag = true; }

bool body_is_removed(body_t *body) { return body->flag; }
