#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// constants
const vector_t WINDOW_MIN = {.x = 0, .y = 0};
const vector_t WINDOW_MAX = {.x = 1000, .y = 500};
const vector_t CENTER = {.x = 500, .y = 250};
const size_t LOCATION_X_MAX = 900;
const size_t LOCATION_X_MIN = 100;
const size_t LOCATION_Y_MAX = 450;
const size_t LOCATION_Y_MIN = 50;

const float PI = M_PI;
const size_t HALF_CIRCLE = 180;
const double ROT_DEGREE_RIGHT = 3 * PI / 2;
const double ROT_DEGREE_LEFT = PI / 2;

const double PELLET_RADIUS = 5;
const double PELLET_MASS = 10.0;
const rgb_color_t PELLET_COLOR = {1.0, 1.0, 1.0};
const rgb_color_t PACMAN_COLOR = {1.0, 0.5, 0.5};

const size_t NUM_PELLETS = 25;
const size_t PELLET_POINTS = 360;
const size_t PELLET_TIME = 2;

const double PACMAN_RADIUS = 30;
const size_t PACMAN_POINTS = 301;
const size_t PACMAN_DEGREE = 29;

const double ACCELERATION = 50.0;

typedef struct state {
  scene_t *pellet_list;
  float time;
} state_t;

body_t *make_pacman(size_t center_x, size_t center_y) {
  size_t num_points = PACMAN_POINTS; // cut out 30 degree indent
  double x;
  double y;

  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);

  vector_t *center_pt = malloc(sizeof(vector_t));
  *center_pt = (vector_t){.x = center_x, .y = center_y};
  list_add(vertices, center_pt);

  // make circle
  for (size_t i = 1; i < num_points; i++) {
    double curr_angle = ((i + PACMAN_DEGREE) * PI) / HALF_CIRCLE;
    x = cos(curr_angle) * PACMAN_RADIUS + center_x;
    y = sin(curr_angle) * PACMAN_RADIUS + center_y;
    vector_t *vec_ptr = malloc(sizeof(vector_t));
    *vec_ptr = (vector_t){.x = x, .y = y};
    list_add(vertices, vec_ptr);
  }

  body_t *pacman = body_init(vertices, PELLET_MASS, PACMAN_COLOR);

  // add center point to verticies
  vector_t *center = malloc(sizeof(vector_t));
  *center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(pacman, *center);

  return pacman;
}

body_t *make_pellet() {
  list_t *pellet_shape = list_init(PELLET_POINTS, free);
  assert(pellet_shape != NULL);

  // randomize position
  double center_x =
      (rand() % (LOCATION_X_MAX - LOCATION_X_MIN + 1)) + LOCATION_X_MIN;
  double center_y =
      (rand() % (LOCATION_Y_MAX - LOCATION_Y_MIN + 1)) + LOCATION_Y_MIN;

  // make circle
  for (size_t i = 1; i < PELLET_POINTS; i++) {
    double curr_angle = (i * PI) / HALF_CIRCLE;
    size_t x = cos(curr_angle) * PELLET_RADIUS + center_x;
    size_t y = sin(curr_angle) * PELLET_RADIUS + center_y;
    vector_t *pnt = malloc(sizeof(vector_t));
    *pnt = (vector_t){.x = x, .y = y};
    list_add(pellet_shape, pnt);
  }

  body_t *pellet = body_init(pellet_shape, PELLET_MASS, PELLET_COLOR);
  return pellet;
}

void move_pacman(char key, key_event_type_t pressed, double time_held,
                 state_t *state) {
  body_t *pacman = scene_get_body(state->pellet_list, 0);
  vector_t *v_velocity = malloc(sizeof(vector_t));
  *v_velocity = body_get_velocity(pacman);
  if (pressed == KEY_PRESSED) {
    if (key == LEFT_ARROW) {
      body_set_fake_rotation(pacman, PI);
      body_set_angle(pacman, PI);
      *v_velocity =
          (vector_t){.x = (-1) * (ACCELERATION * time_held), .y = 0.0};
      body_set_velocity(pacman, *v_velocity);
    } else if (key == RIGHT_ARROW) {
      body_set_fake_rotation(pacman, 0);
      body_set_angle(pacman, 0);
      *v_velocity = (vector_t){.x = (1) * (ACCELERATION * time_held), .y = 0.0};
      body_set_velocity(pacman, *v_velocity);
    } else if (key == DOWN_ARROW) {
      body_set_fake_rotation(pacman, ROT_DEGREE_RIGHT);
      body_set_angle(pacman, ROT_DEGREE_RIGHT);
      *v_velocity =
          (vector_t){.x = 0.0, .y = (-1) * (ACCELERATION * time_held)};
      body_set_velocity(pacman, *v_velocity);
    } else if (key == UP_ARROW) {
      body_set_fake_rotation(pacman, ROT_DEGREE_LEFT);
      body_set_angle(pacman, ROT_DEGREE_LEFT);
      *v_velocity = (vector_t){.x = 0.0, .y = (1) * (ACCELERATION * time_held)};
      body_set_velocity(pacman, *v_velocity);
    }
  }
}

void wrap_around(state_t *state) {
  if (body_get_centroid(scene_get_body(state->pellet_list, 0)).x -
          PACMAN_RADIUS >
      WINDOW_MAX.x) {
    body_set_centroid(
        scene_get_body(state->pellet_list, 0),
        (vector_t){
            .x = WINDOW_MIN.x,
            .y = body_get_centroid(scene_get_body(state->pellet_list, 0)).y});
  }
  if (body_get_centroid(scene_get_body(state->pellet_list, 0)).x +
          PACMAN_RADIUS <
      WINDOW_MIN.x) {
    body_set_centroid(
        scene_get_body(state->pellet_list, 0),
        (vector_t){
            .x = WINDOW_MAX.x,
            .y = body_get_centroid(scene_get_body(state->pellet_list, 0)).y});
  }
  if (body_get_centroid(scene_get_body(state->pellet_list, 0)).y -
          PACMAN_RADIUS >
      WINDOW_MAX.y) {
    body_set_centroid(
        scene_get_body(state->pellet_list, 0),
        (vector_t){
            .x = body_get_centroid(scene_get_body(state->pellet_list, 0)).x,
            .y = WINDOW_MIN.y});
  }
  if (body_get_centroid(scene_get_body(state->pellet_list, 0)).y +
          PACMAN_RADIUS <
      WINDOW_MIN.y) {
    body_set_centroid(
        scene_get_body(state->pellet_list, 0),
        (vector_t){
            .x = body_get_centroid(scene_get_body(state->pellet_list, 0)).x,
            .y = WINDOW_MAX.y});
  }
}

// spawn pellets according to time interval
void spawn_pellets(state_t *state) {
  if (state->time > PELLET_TIME) {
    scene_add_body(state->pellet_list, make_pellet());
    state->time = 0;
  }
}

// eat pellets
void eat_pellets(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->pellet_list); i++) {
    // find distance between point and center, if it is =< radius, eat
    double x_distance =
        body_get_centroid(scene_get_body(state->pellet_list, 0)).x -
        body_get_centroid(scene_get_body(state->pellet_list, i)).x;
    double y_distance =
        body_get_centroid(scene_get_body(state->pellet_list, 0)).y -
        body_get_centroid(scene_get_body(state->pellet_list, i)).y;
    double hypot = sqrt(pow(x_distance, 2) + pow(y_distance, 2));
    if (hypot <= PACMAN_RADIUS) {
      scene_remove_body_fake(state->pellet_list, i);
      i--;
    }
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));

  // create pacman
  body_t *pacman = make_pacman(CENTER.x, CENTER.y);

  // create initial pellets
  state->pellet_list = scene_init();
  state->time = 0.0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  scene_add_body(state->pellet_list, pacman);
  for (int i = 1; i < NUM_PELLETS; i++) {
    scene_add_body(state->pellet_list, make_pellet());
  }
  sdl_on_key(move_pacman);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time = state->time + dt;

  scene_tick(state->pellet_list, dt);
  if (!sdl_is_done(state)) {
    sdl_draw_polygon(body_get_vertices(scene_get_body(state->pellet_list, 0)),
                     PACMAN_COLOR);
    for (size_t i = 1; i < scene_bodies(state->pellet_list); i++) {
      sdl_draw_polygon(body_get_vertices(scene_get_body(state->pellet_list, i)),
                       PELLET_COLOR);
    }
    sdl_show();
  }
  spawn_pellets(state);
  eat_pellets(state);
  wrap_around(state);
}

void emscripten_free(state_t *state) {
  scene_free(state->pellet_list);
  free(state);
}
