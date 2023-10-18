#include "forces.h"
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
const double TWO_PI_N = 2.0 * M_PI;
const vector_t WINDOW = {.x = 1000, .y = 500};
const vector_t WINDOW_MAX = {.x = 1000, .y = 500};
const vector_t WINDOW_MIN = {.x = 0, .y = 0};

const vector_t WINDOW_BODY_MAX = {.x = 950, .y = 450};
const vector_t WINDOW_BODY_MIN = {.x = 50, .y = 50};

const size_t NUM_BODIES = 40;
const double MAX_MASS = 150.0;
const double MIN_MASS = 100.0;
const double NUM_POINTS = 8;

const double MASS_CONSTANT = 20;
const double G = 100.0;

// state
typedef struct state {
  scene_t *body_list;
  float time;
} state_t;

// creates and returns a (body_t *) star with four points
body_t *make_four_star(double center_x, double center_y, double mass,
                       double radius) {
  double x;
  double y;

  list_t *vertices = list_init(NUM_POINTS, free);

  // loops through number of points we want to create for each star
  for (size_t i = 0; i <= NUM_POINTS - 1; i += 2) {
    // outside points
    x = radius * cos((TWO_PI_N * (i / 2)) / (NUM_POINTS / 2)) + center_x;
    y = radius * sin((TWO_PI_N * (i / 2)) / (NUM_POINTS / 2)) + center_y;

    vector_t *temp = malloc(sizeof(vector_t));
    *temp = (vector_t){x, y};
    list_add(vertices, temp);

    // inside points
    x = (radius / 2) * cos(TWO_PI_N * (i + 1) / NUM_POINTS) + center_x;
    y = (radius / 2) * sin(TWO_PI_N * (i + 1) / NUM_POINTS) + center_y;

    vector_t *tem = malloc(sizeof(vector_t));
    *tem = (vector_t){x, y};
    list_add(vertices, tem);
  }
  // random color
  rgb_color_t color = {.r = (float)rand() / (float)RAND_MAX,
                       .g = (float)rand() / (float)RAND_MAX,
                       .b = (float)rand() / (float)RAND_MAX};
  body_t *diamond = body_init(vertices, mass, color);
  body_set_centroid(diamond, (vector_t){.x = center_x, .y = center_y});
  return diamond;
}

// initializes the state and creates the list of bodies
state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->body_list = scene_init();
  state->time = 0.0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  // loops through number of bodies to create each one
  for (size_t i = 0; i < NUM_BODIES; i++) {
    double center_x =
        fmod(rand(), (WINDOW_BODY_MAX.x + 1 - WINDOW_BODY_MIN.x)) +
        WINDOW_BODY_MIN.x;
    double center_y =
        fmod(rand(), (WINDOW_BODY_MAX.y + 1 - WINDOW_BODY_MIN.y)) +
        WINDOW_BODY_MIN.y;
    double mass = fmod(rand(), (MAX_MASS + 1 - MIN_MASS)) + MIN_MASS;
    double radius = mass / MASS_CONSTANT;
    scene_add_body(state->body_list,
                   make_four_star(center_x, center_y, mass, radius));
  }
  // nested for loop to apply forces to each pair of bodies
  for (size_t i = 0; i < NUM_BODIES; i++) {
    for (size_t j = i + 1; j < NUM_BODIES; j++) {
      create_newtonian_gravity(state->body_list, G,
                               scene_get_body(state->body_list, i),
                               scene_get_body(state->body_list, j));
    }
  }
  return state;
}

// updates the screen as forces are applied to the bodies and bodies move
void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time = state->time + dt;

  scene_tick(state->body_list, dt);

  // for loop running through the list of bodies to redraw the bodies
  if (!sdl_is_done(state)) {
    sdl_render_scene(state->body_list);
  }
  sdl_show();
}

// frees the state
void emscripten_free(state_t *state) {
  scene_free(state->body_list);
  free(state);
}
