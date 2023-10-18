#include "collision.h"
#include "forces.h"
#include "list.h"
#include "polygon.h"
#include "sdl_wrapper.h"
#include "state.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// constants
const vector_t WINDOW_MIN = {.x = 0, .y = 0};
const vector_t WINDOW_MAX = {.x = 1000, .y = 500};
const vector_t WINDOW_CENTER = {.x = 500, .y = 250};
const vector_t WINDOW_BODY_MAX = {.x = 950, .y = 450};
const vector_t WINDOW_BODY_MIN = {.x = 50, .y = 50};
const vector_t WINDOW_TOP_LEFT = {.x = 0, .y = 500};
const vector_t WINDOW_BOTTOM_RIGHT = {.x = 1000, .y = 0};
const size_t NUM_ROWS = 3;
const size_t NUM_COLUMNS = 10;
bool up_star = false;
bool up_moon = false;
// miscellaneous constants
const float PI = M_PI;
const double G = 6.67430e-11;
const double EARTH_MASS = 5.97219e24;
const double EARTH_RADIUS = -6378100;
// platform constants
const double PLAT_LENGTH = 60;
const double PLAT_HEIGHT = 10;
const double PLAT_MASS = 60;
const rgb_color_t PLAT_COLOR = {1.0, 1.0, 1.0};
const double ACCELERATION = 50.0;

typedef struct state {
  // make it so that index 0 is grav bar, 1 is stargirl, 2 is moon boy, and rest
  // are platforms
  scene_t *grav_player_list;
  scene_t *platforms_list;
  scene_t *trampoline_list;
  double time;
} state_t;

body_t *make_planet(state_t *state, double center_x, double center_y) {
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (5 / 2), .y = center_y + (5 / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (5 / 2), .y = center_y + (5 / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (5 / 2), .y = center_y - (5 / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (5 / 2), .y = center_y - (5 / 2)};
  list_add(vertices, pt4);

  body_t *bar =
      body_init_with_info(vertices, EARTH_MASS, PLAT_COLOR, "bar", NULL);
  return bar;
}

body_t *make_player(state_t *state, double center_x, double center_y) {
  // make list & add vertices of rectangle
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (50 / 2), .y = center_y + (68.5 / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (50 / 2), .y = center_y + (68.5 / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (50 / 2), .y = center_y - (68.5 / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (50 / 2), .y = center_y - (68.5 / 2)};
  list_add(vertices, pt4);

  rgb_color_t color = (rgb_color_t){1.0, 1.0, 1.0};

  body_t *rectangle =
      body_init_with_prev_grav_info(false, false, vertices, 50, color);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(rectangle, center);
  vector_t vel = (vector_t){.x = 0, .y = 5};
  vel = vec_negate(vel);
  body_set_velocity(rectangle, vel);
  create_newtonian_gravity(state->grav_player_list, G,
                           scene_get_body(state->grav_player_list, 0),
                           rectangle);
  return rectangle;
}

body_t *make_platform(state_t *state, double center_x, double center_y,
                      double length) {
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (length / 2),
                    .y = center_y + (PLAT_HEIGHT / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (length / 2),
                    .y = center_y + (PLAT_HEIGHT / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (length / 2),
                    .y = center_y - (PLAT_HEIGHT / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (length / 2),
                    .y = center_y - (PLAT_HEIGHT / 2)};
  list_add(vertices, pt4);

  body_t *platform = body_init_with_prev_grav_info(false, false, vertices,
                                                   PLAT_MASS, PLAT_COLOR);

  return platform;
}

body_t *make_trampoline(state_t *state, double center_x, double center_y) {
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (40 / 2), .y = center_y + (20 / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (40 / 2), .y = center_y + (20 / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (40 / 2), .y = center_y - (20 / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (40 / 2), .y = center_y - (20 / 2)};
  list_add(vertices, pt4);

  body_t *rectangle = body_init_with_prev_grav_info(false, false, vertices,
                                                    PLAT_MASS, PLAT_COLOR);

  return rectangle;
}

void hitting_sides(state_t *state, body_t *body_player) {
  if (body_get_centroid(body_player).x + 25 >= 1000) {
    body_set_centroid(
        body_player,
        (vector_t){.x = 975, .y = body_get_centroid(body_player).y});
  }
  if (body_get_centroid(body_player).x - 25 <= 0) {
    body_set_centroid(
        body_player,
        (vector_t){.x = 25, .y = body_get_centroid(body_player).y});
  }
  if (body_get_centroid(body_player).y <= (50.0 * 585 / 427) / 2) {
    body_set_centroid(body_player,
                      (vector_t){.x = body_get_centroid(body_player).x,
                                 .y = (50.0 * 585 / 427) / 2});
  }
}

void move_characters(uint8_t *temp, key_event_type_t pressed, double time_held,
                     state_t *state) {
  body_t *girl = scene_get_body(state->grav_player_list, 1);
  body_t *boy =
      scene_get_body(state->grav_player_list, 2); // LATER MAKE BOY 1 GIRL 2
  vector_t *v_velocity = malloc(sizeof(vector_t));
  if (temp[4] && temp[26]) { // a & w
    *v_velocity = (vector_t){.x = (-1) * (ACCELERATION), .y = 20.0};
    body_set_velocity(girl, *v_velocity);
  } else if (temp[7] && temp[26]) { // d & w
    *v_velocity = (vector_t){.x = (1) * (ACCELERATION), .y = 20.0};
    body_set_velocity(girl, *v_velocity);
  }
  if (temp[4]) { // a
    *v_velocity =
        (vector_t){.x = (-1) * (ACCELERATION), .y = body_get_velocity(girl).y};
    body_set_velocity(girl, *v_velocity);
  }
  if (temp[7]) { // d
    *v_velocity =
        (vector_t){.x = (1) * (ACCELERATION), .y = body_get_velocity(girl).y};
    body_set_velocity(girl, *v_velocity);
  }
  if (temp[26] && up_star == false) { // w
    up_star = true;
    // body_add_impulse(girl, (vector_t){0, 50000});
    *v_velocity = (vector_t){.x = 0.0, .y = 20.0};
    body_set_velocity(girl, *v_velocity);
  } else {
    up_star = false;
  }
  if (!temp[4] && !temp[26] && !temp[7]) {
    body_set_velocity(girl, (vector_t){.x = 0, .y = body_get_velocity(girl).y});
  }
  // temp[22] for s (shoot)
  // if (temp[80] && temp[82]) { //left and up
  //  *v_velocity = (vector_t){.x = (-1) * (ACCELERATION), .y = 30.0};
  //  body_set_velocity(boy, *v_velocity);
  //}
  // else if (temp[79] && temp[82]) { //right and up
  //  *v_velocity = (vector_t){.x = (1) * (ACCELERATION), .y = 30.0};
  //  body_set_velocity(boy, *v_velocity);
  //}
  if (temp[80]) { // left
    *v_velocity =
        (vector_t){.x = (-1) * (ACCELERATION), .y = body_get_velocity(boy).y};
    body_set_velocity(boy, *v_velocity);
  }
  if (temp[79]) { // right
    *v_velocity =
        (vector_t){.x = (1) * (ACCELERATION), .y = body_get_velocity(boy).y};
    body_set_velocity(boy, *v_velocity);
  }
  if (temp[82] && up_moon == false) { // up
    up_moon = true;
    *v_velocity = (vector_t){.x = 0.0, .y = 20.0};
    body_set_velocity(boy, *v_velocity);
  } else {
    up_moon = false;
  }
  // temp[81] for down (defend)
  if (!temp[80] && !temp[79] && !temp[82]) {
    body_set_velocity(boy, (vector_t){.x = 0, .y = body_get_velocity(boy).y});
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->grav_player_list = scene_init();
  state->platforms_list = scene_init();
  state->trampoline_list = scene_init();
  state->time = 0.0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  scene_add_body(state->trampoline_list, make_trampoline(state, 500, 10));
  scene_add_body(state->grav_player_list,
                 make_planet(state, 500, EARTH_RADIUS));
  scene_add_body(state->grav_player_list, make_player(state, 500, 400));
  scene_add_body(state->grav_player_list, make_player(state, 150, 250));
  for (size_t i = 0; i < 5; i++) {
    size_t x = (170 * i) + 140;
    if (i == 0 || i == 4) {
      size_t y = 200;
      scene_add_body(state->platforms_list, make_platform(state, x, y, 50));
    }
    if (i == 1 || i == 3) {
      size_t y_1 = 200 + 70;
      size_t y_2 = 200 - 70;
      scene_add_body(state->platforms_list, make_platform(state, x, y_1, 50));
      scene_add_body(state->platforms_list, make_platform(state, x, y_2, 50));
    }
    if (i == 2) {
      size_t y_1 = 200 + 70 * 2;
      size_t y_2 = 200 - 70 * 2;
      scene_add_body(state->platforms_list, make_platform(state, x, y_1, 50));
      scene_add_body(state->platforms_list, make_platform(state, x, y_2, 50));
    }
  }
  scene_add_body(state->platforms_list, make_platform(state, 50, 440, 200));
  scene_add_body(state->platforms_list, make_platform(state, 950, 440, 200));

  sdl_on_key(move_characters);
  return state;
}

void reset_collision(state_t *state, body_t *body_player, body_t *platform) {
  vector_t vel = (vector_t){.x = body_get_velocity(body_player).x, .y = 0};
  body_set_velocity(body_player, vel);
  // set flag for gravity removal
  body_set_grav(body_player, true);
  // vector_t new_pos = (vector_t) {.x = body_get_centroid(body).x,
  //  .y = body_get_centroid(body).y + 10};
  // body_set_centroid(scene_get_body(state->platforms_list, 1), new_pos);
  // remember to reset gravity removal flag
  vector_t vel2 = (vector_t){.x = 0, .y = body_get_velocity(body_player).y};
  if (body_get_prev_collision(platform) == false) {
    body_set_velocity(body_player, vel2);
  }
  body_set_prev_collision(platform, true);
}

void start_grav(state_t *state, body_t *body_player, body_t *platform) {
  body_set_grav(body_player, false);
  vector_t vel = (vector_t){.x = body_get_velocity(body_player).x, .y = 5};
  body_set_velocity(body_player, vel);
  body_set_prev_collision(platform, false);
}

void check_collision(state_t *state, body_t *body_player, body_t *platform,
                     bool key_up) {
  // make new flag for prev collision, if true and find collision and holding
  // key up, dont reset collision if find collision otherwise reset collision
  if (find_collision(body_get_vertices(body_player),
                     body_get_vertices(platform))
          .collided) {
    // if in boundary of platform
    if (body_get_centroid(body_player).x - 18 <
            body_get_centroid(platform).x + PLAT_LENGTH / 2 &&
        body_get_centroid(body_player).x + 18 >
            body_get_centroid(platform).x - PLAT_LENGTH / 2) {
      if (body_get_centroid(body_player).y < body_get_centroid(platform).y) {
        vector_t new_pos = (vector_t){body_get_centroid(body_player).x,
                                      body_get_centroid(body_player).y - 1};
        body_set_centroid(body_player, new_pos);
        //   printf("%s\n", "collides in boundary, hits from bottom");
      } else if (key_up && body_get_prev_collision(platform)) {
        body_set_grav(body_player, false);
        body_set_prev_collision(platform, false);
        //  printf("%s\n", "jump, on platform");
      } else {
        reset_collision(state, body_player, platform);
        //     printf("%s\n", "in boundary, doesnt jump, on platform");
      }
    } else {
      // if out of boundary of platform
      if (key_up && body_get_prev_collision(platform)) {
        //  printf("%s\n", "jump, off platform");
        start_grav(state, body_player, platform);
      } else {
        // if its colliding and its falling (colliding from the side)
        body_set_grav(body_player, false);
        body_set_prev_collision(platform, false);
        if (body_get_centroid(body_player).x - 25 <
                body_get_centroid(platform).x + PLAT_LENGTH / 2 &&
            body_get_centroid(body_player).x + 25 >
                body_get_centroid(platform).x + PLAT_LENGTH / 2) {
          vector_t new_pos =
              (vector_t){.x = body_get_centroid(body_player).x + 1,
                         .y = body_get_centroid(body_player).y};
          body_set_centroid(body_player, new_pos);
        }
        if (body_get_centroid(body_player).x + 25 >
                body_get_centroid(platform).x - PLAT_LENGTH / 2 &&
            body_get_centroid(body_player).x - 25 <
                body_get_centroid(platform).x - PLAT_LENGTH / 2) {
          vector_t new_pos =
              (vector_t){.x = body_get_centroid(body_player).x - 1,
                         .y = body_get_centroid(body_player).y};
          body_set_centroid(body_player, new_pos);
        }
        // printf("%s\n", "out of boundary, doesnt jump, off platform");
      }
    }
  }
}

void trampoline_collision(state_t *state) {
  body_t *player1 = scene_get_body(state->grav_player_list, 1);
  body_t *player2 = scene_get_body(state->grav_player_list, 2);
  double elasticity = 1.0;
  for (size_t i = 0; i < scene_bodies(state->trampoline_list); i++) {
    create_physics_collision(state->trampoline_list, elasticity, player1,
                             scene_get_body(state->trampoline_list, i));
  }
  for (size_t i = 0; i < scene_bodies(state->trampoline_list); i++) {
    create_physics_collision(state->trampoline_list, elasticity, player2,
                             scene_get_body(state->trampoline_list, i));
  }
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  bool test;
  for (size_t i = 0; i < scene_bodies(state->platforms_list); i++) {
    check_collision(state, scene_get_body(state->grav_player_list, 1),
                    scene_get_body(state->platforms_list, i), up_star);
    check_collision(state, scene_get_body(state->grav_player_list, 2),
                    scene_get_body(state->platforms_list, i), up_moon);
    if (find_collision(
            body_get_vertices(scene_get_body(state->grav_player_list, 1)),
            body_get_vertices(scene_get_body(state->platforms_list, i)))
            .collided) {
      test = false;
    }
  }
  if (test) {
    body_set_grav(scene_get_body(state->grav_player_list, 1), false);
  }

  hitting_sides(state, scene_get_body(state->grav_player_list, 1));
  hitting_sides(state, scene_get_body(state->grav_player_list, 2));
  trampoline_collision(state);

  scene_tick(state->grav_player_list, dt);
  // sdl_draw_sprite("assets/background.png", WINDOW_CENTER, WINDOW_MAX);

  for (size_t i = 0; i < scene_bodies(state->grav_player_list); i++) {
    if (i != 1 && i != 2) {
      sdl_draw_polygon(
          body_get_vertices(scene_get_body(state->grav_player_list, i)),
          body_get_color(scene_get_body(state->grav_player_list, i)));
    }
  }

  scene_tick(state->platforms_list, dt);
  for (size_t i = 0; i < scene_bodies(state->platforms_list); i++) {
    sdl_draw_polygon(
        body_get_vertices(scene_get_body(state->platforms_list, i)),
        body_get_color(scene_get_body(state->platforms_list, i)));
  }

  scene_tick(state->trampoline_list, dt);
  for (size_t i = 0; i < scene_bodies(state->trampoline_list); i++) {
    sdl_draw_polygon(
        body_get_vertices(scene_get_body(state->trampoline_list, i)),
        body_get_color(scene_get_body(state->trampoline_list, i)));
  }

  vector_t center_of_sprite =
      body_get_centroid(scene_get_body(state->grav_player_list, 2));
  double old_y = center_of_sprite.y;
  center_of_sprite.y = old_y - (2 * (old_y - 250));
  sdl_draw_sprite("assets/moon_boy.png", center_of_sprite,
                  (vector_t){50.0, 50.0 * 585 / 427});

  vector_t center_of_sprite_star =
      body_get_centroid(scene_get_body(state->grav_player_list, 1));
  double old_y2 = center_of_sprite_star.y;
  center_of_sprite_star.y = old_y2 - (2 * (old_y2 - 250));
  sdl_draw_sprite("assets/star_girl.png", center_of_sprite_star,
                  (vector_t){50.0, 50.0 * 585 / 427});

  sdl_show();
}

void emscripten_free(state_t *state) {
  scene_free(state->grav_player_list);
  scene_free(state->platforms_list);
  scene_free(state->trampoline_list);
  free(state);
}
