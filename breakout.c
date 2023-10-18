#include "collision.h"
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
// miscellaneous constants
const float PI = M_PI;
const size_t HALF_CIRCLE = 180;
// brick constants
const double BRICK_LENGTH = 89;
const double BRICK_HEIGHT = 18;
const double BRICK_MASS = INFINITY;
const size_t BRICK_X_INCREMENT = 55;
const size_t BRICK_Y_INCREMENT = 15;
const size_t NUM_BRICKS = 30;
rgb_color_t BRICK_COLOR = (rgb_color_t){255 / 255, 128 / 255, 128 / 255};
rgb_color_t BRICK_PREV_COLOR = (rgb_color_t){255 / 255, 128 / 255, 128 / 255};
size_t R_COUNTER = 3;
size_t G_COUNTER = 5;
size_t B_COUNTER = 1;
// main body constants
const double MAIN_BODY_LENGTH = 100;
const double MAIN_BODY_HEIGHT = 30;
const double MAIN_BODY_MASS = INFINITY;
rgb_color_t MAIN_BODY_COLOR = {1.0, 1.0, 1.0};
const double HIT_SIDE_POS_Y = 50;
const vector_t MAIN_VELOCITY = {.x = 300, .y = 0};
// ball constants
const double BALL_RADIUS = 15;
const double BALL_MASS = 0.000001;
const vector_t BALL_VEL = {.x = 100, .y = 100};
rgb_color_t BALL_COLOR = {1.0, 1.0, .9};
// wall constants
const double WALL_MASS = INFINITY;
const double WALL_WIDTH = 20;
rgb_color_t WALL_COLOR = {0.0, 0.0, 0.0}; // 1, 0.6, 1

typedef struct state {
  scene_t *body_list;
  double time;
  size_t counter;
} state_t;

body_t *make_brick(size_t center_x, size_t center_y, rgb_color_t color) {
  // make list & add vertices of rectangle
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (BRICK_LENGTH / 2),
                    .y = center_y + (BRICK_HEIGHT / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (BRICK_LENGTH / 2),
                    .y = center_y + (BRICK_HEIGHT / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (BRICK_LENGTH / 2),
                    .y = center_y - (BRICK_HEIGHT / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (BRICK_LENGTH / 2),
                    .y = center_y - (BRICK_HEIGHT / 2)};
  list_add(vertices, pt4);

  body_t *brick =
      body_init_with_info(vertices, BRICK_MASS, color, "brick", NULL);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(brick, center);
  return brick;
}

body_t *make_main_body(size_t center_x, size_t center_y) {
  // make list & add vertices of rectangle
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (MAIN_BODY_LENGTH / 2),
                    .y = center_y + (MAIN_BODY_HEIGHT / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (MAIN_BODY_LENGTH / 2),
                    .y = center_y + (MAIN_BODY_HEIGHT / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (MAIN_BODY_LENGTH / 2),
                    .y = center_y - (MAIN_BODY_HEIGHT / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (MAIN_BODY_LENGTH / 2),
                    .y = center_y - (MAIN_BODY_HEIGHT / 2)};
  list_add(vertices, pt4);

  body_t *main_body = body_init_with_info(vertices, MAIN_BODY_MASS,
                                          MAIN_BODY_COLOR, "main", NULL);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(main_body, center);
  return main_body;
}

body_t *make_ball(size_t center_x, size_t center_y) {
  size_t num_points = 360;
  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);

  double x;
  double y;

  // make circle
  for (size_t i = 1; i < num_points; i++) {
    double curr_angle = ((i + 29) * PI) / HALF_CIRCLE;
    x = cos(curr_angle) * BALL_RADIUS + center_x;
    y = sin(curr_angle) * BALL_RADIUS + center_y;
    vector_t *vec_ptr = malloc(sizeof(vector_t));
    *vec_ptr = (vector_t){.x = x, .y = y};
    list_add(vertices, vec_ptr);
  }

  body_t *ball =
      body_init_with_info(vertices, BALL_MASS, BALL_COLOR, "ball", NULL);

  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(ball, center);
  body_set_velocity(ball, BALL_VEL);
  return ball;
}

body_t *make_vertical_wall(size_t center_x, size_t center_y) {
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (WALL_WIDTH / 2),
                    .y = center_y + (WINDOW_MAX.y / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (WALL_WIDTH / 2),
                    .y = center_y + (WINDOW_MAX.y / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (WALL_WIDTH / 2),
                    .y = center_y - (WINDOW_MAX.y / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (WALL_WIDTH / 2),
                    .y = center_y - (WINDOW_MAX.y / 2)};
  list_add(vertices, pt4);

  body_t *vertical_wall =
      body_init_with_info(vertices, WALL_MASS, WALL_COLOR, "wall", NULL);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(vertical_wall, center);
  return vertical_wall;
}

body_t *make_horizontal_wall(size_t center_x, size_t center_y) {
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (WINDOW_MAX.x / 2),
                    .y = center_y + (WALL_WIDTH / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (WINDOW_MAX.x / 2),
                    .y = center_y + (WALL_WIDTH / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (WINDOW_MAX.x / 2),
                    .y = center_y - (WALL_WIDTH / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (WINDOW_MAX.x / 2),
                    .y = center_y - (WALL_WIDTH / 2)};
  list_add(vertices, pt4);

  body_t *horizontal_wall =
      body_init_with_info(vertices, WALL_MASS, WALL_COLOR, "wall", NULL);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(horizontal_wall, center);
  return horizontal_wall;
}

// This function tests if the user hits the left or right side of the window
/*void hitting_sides(state_t *state) {
  body_t *user = scene_get_body(state->body_list, 1);
  double min_centroid = WINDOW_BODY_MAX.x - (MAIN_BODY_LENGTH / 2) - 2;
  if (body_get_centroid(user).x + (MAIN_BODY_LENGTH / 2) > WINDOW_BODY_MAX.x) {
    body_set_centroid(user, (vector_t){.x = min_centroid, .y = HIT_SIDE_POS_Y});
  }
  double max_centroid = WINDOW_BODY_MIN.x + (MAIN_BODY_LENGTH / 2) + 2;
  if (body_get_centroid(user).x - (MAIN_BODY_LENGTH / 2) < WINDOW_BODY_MIN.x) {
    body_set_centroid(user, (vector_t){.x = max_centroid, .y = HIT_SIDE_POS_Y});
  }
}
*/
// arrows to main_body moving

void shift_all_rows(state_t *state) { // printf("%s\n", "test");
  for (size_t i = 5; i < scene_bodies(state->body_list); i++) {
    double x = body_get_centroid(scene_get_body(state->body_list, i)).x;
    double y = body_get_centroid(scene_get_body(state->body_list, i)).y -
               BRICK_HEIGHT - BRICK_Y_INCREMENT / 2;
    vector_t new_pos = (vector_t){.x = x, .y = y};
    body_set_centroid(scene_get_body(state->body_list, i), new_pos);
  }
}

void make_singular_row(state_t *state) {
  rgb_color_t BRICK_COLOR = (rgb_color_t){1.0, 0.5, 0.5};
  rgb_color_t BRICK_PREV_COLOR = (rgb_color_t){1.0, 0.5, 0.5};
  size_t R_COUNTER = 3;
  size_t G_COUNTER = 5;
  size_t B_COUNTER = 1;

  double x = WINDOW_MIN.x + WINDOW_BODY_MIN.x;
  double y = WINDOW_MAX.y - 15;
  double elasticity = 1.0;
  size_t j = scene_bodies(state->body_list);
  for (size_t i = 0; i < NUM_COLUMNS; i++) {
    scene_add_body(state->body_list,
                   make_brick(x - (BRICK_LENGTH / 2) * i, y, BRICK_COLOR));
    create_half_destructive_collision(state->body_list, elasticity,
                                      scene_get_body(state->body_list, j),
                                      scene_get_body(state->body_list, 0));
    j++;
    x += (BRICK_LENGTH + BRICK_X_INCREMENT);

    if (BRICK_COLOR.r == 0.75 && BRICK_PREV_COLOR.r == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.r = 0.5;
    } else if (BRICK_COLOR.r == 0.75 && BRICK_PREV_COLOR.r == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.r = 1.0;
    }

    if (BRICK_COLOR.g == 0.75 && BRICK_PREV_COLOR.g == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.g = 0.5;
    } else if (BRICK_COLOR.g == 0.75 && BRICK_PREV_COLOR.g == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.g = 1.0;
    }

    if (BRICK_COLOR.b == 0.75 && BRICK_PREV_COLOR.b == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.b = 0.5;
    } else if (BRICK_COLOR.b == 0.75 && BRICK_PREV_COLOR.b == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.b = 1.0;
    }

    BRICK_PREV_COLOR = BRICK_COLOR;

    if (R_COUNTER == 5) {
      BRICK_COLOR.r = 0.75;
      R_COUNTER = 0;
    } else if (G_COUNTER == 5) {
      BRICK_COLOR.g = 0.75;
      G_COUNTER = 0;
    } else if (B_COUNTER == 5) {
      BRICK_COLOR.b = 0.75;
      B_COUNTER = 0;
    }

    R_COUNTER++;
    G_COUNTER++;
    B_COUNTER++;
  }
}

void spawn_bricks(state_t *state) {
  rgb_color_t BRICK_COLOR = (rgb_color_t){1.0, 0.5, 0.5};
  rgb_color_t BRICK_PREV_COLOR = (rgb_color_t){1.0, 0.5, 0.5};
  size_t R_COUNTER = 3;
  size_t G_COUNTER = 5;
  size_t B_COUNTER = 1;

  double x = WINDOW_MIN.x + WINDOW_BODY_MIN.x;
  double y = WINDOW_MAX.y - 15;
  for (int i = 0; i < NUM_COLUMNS; i++) {
    scene_add_body(state->body_list,
                   make_brick(x - (BRICK_LENGTH / 2) * i, y, BRICK_COLOR));
    scene_add_body(state->body_list,
                   make_brick(x - (BRICK_LENGTH / 2) * i,
                              y - (BRICK_HEIGHT / 2) - BRICK_Y_INCREMENT,
                              BRICK_COLOR));
    scene_add_body(state->body_list,
                   make_brick(x - (BRICK_LENGTH / 2) * i,
                              y - ((BRICK_HEIGHT / 2) + BRICK_Y_INCREMENT) * 2,
                              BRICK_COLOR));
    x += (BRICK_LENGTH + BRICK_X_INCREMENT);

    if (BRICK_COLOR.r == 0.75 && BRICK_PREV_COLOR.r == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.r = 0.5;
    } else if (BRICK_COLOR.r == 0.75 && BRICK_PREV_COLOR.r == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.r = 1.0;
    }

    if (BRICK_COLOR.g == 0.75 && BRICK_PREV_COLOR.g == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.g = 0.5;
    } else if (BRICK_COLOR.g == 0.75 && BRICK_PREV_COLOR.g == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.g = 1.0;
    }

    if (BRICK_COLOR.b == 0.75 && BRICK_PREV_COLOR.b == 1.0) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.b = 0.5;
    } else if (BRICK_COLOR.b == 0.75 && BRICK_PREV_COLOR.b == 0.5) {
      BRICK_PREV_COLOR = BRICK_COLOR;
      BRICK_COLOR.b = 1.0;
    }

    BRICK_PREV_COLOR = BRICK_COLOR;

    if (R_COUNTER == 5) {
      BRICK_COLOR.r = 0.75;
      R_COUNTER = 0;
    } else if (G_COUNTER == 5) {
      BRICK_COLOR.g = 0.75;
      G_COUNTER = 0;
    } else if (B_COUNTER == 5) {
      BRICK_COLOR.b = 0.75;
      B_COUNTER = 0;
    }

    R_COUNTER++;
    G_COUNTER++;
    B_COUNTER++;
  }
}

// moves user on key press
void move_main_body(char key, key_event_type_t pressed, double time_held,
                    state_t *state) {
  body_t *user = scene_get_body(state->body_list, 1);
  if (pressed == KEY_PRESSED) {
    if (key == LEFT_ARROW) {
      body_set_velocity(user, vec_multiply(-1, MAIN_VELOCITY));
      // (state);
    } else if (key == RIGHT_ARROW) {
      body_set_velocity(user, MAIN_VELOCITY);
      //  hitting_sides(state);
    }
  } else {
    body_set_velocity(user, VEC_ZERO);
  }
}

void ball_collision(state_t *state) {
  body_t *ball = scene_get_body(state->body_list, 0);
  double elasticity = 1.0;
  for (size_t i = 1; i < 5; i++) {
    create_physics_collision(state->body_list, elasticity, ball,
                             scene_get_body(state->body_list, i));
  }
  for (size_t i = 5; i < scene_bodies(state->body_list); i++) {
    create_half_destructive_collision(state->body_list, elasticity,
                                      scene_get_body(state->body_list, i),
                                      ball);
  }
}

void check_hit_bottom(state_t *state) {
  body_t *ball = scene_get_body(state->body_list, 0);
  if (body_get_centroid(ball).y - (MAIN_BODY_HEIGHT / 2) <= WINDOW_MIN.y) {
    for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
      body_remove(scene_get_body(state->body_list, i));
    }
    state->counter = 0;
    // first make the ball so that it is at index 0
    scene_add_body(
        state->body_list,
        make_ball(WINDOW_CENTER.x,
                  WINDOW_MIN.y + 100)); // MAIN_BODY_HEIGHT + BALL_RADIUS + 5));
    // then  make the paddle so that it is at index 1
    scene_add_body(state->body_list,
                   make_main_body(WINDOW_CENTER.x, HIT_SIDE_POS_Y));
    // then make the three walls
    scene_add_body(state->body_list, make_vertical_wall(WINDOW_MIN.x, 250));
    scene_add_body(state->body_list,
                   make_vertical_wall(WINDOW_MAX.x + (WALL_WIDTH / 2), 250));
    scene_add_body(state->body_list,
                   make_horizontal_wall(500, WINDOW_MAX.y + (WALL_WIDTH / 2)));
    spawn_bricks(state);
    // ball_collision(state);
    for (size_t i = 39; i < scene_bodies(state->body_list); i++) {
      create_half_destructive_collision(
          state->body_list, 1.0, scene_get_body(state->body_list, i), ball);
    }
  }
}

void check_hit_walls(state_t *state) {
  if (body_get_centroid(scene_get_body(state->body_list, 1)).x <=
      WINDOW_MIN.x) {
    vector_t new_pos = (vector_t){
        .x = MAIN_BODY_LENGTH,
        .y = body_get_centroid(scene_get_body(state->body_list, 1)).y};
    body_set_centroid(scene_get_body(state->body_list, 1), new_pos);
  }
  if (body_get_centroid(scene_get_body(state->body_list, 1)).x >=
      WINDOW_MAX.x) {
    vector_t new_pos = (vector_t){
        .x = WINDOW_MAX.x - MAIN_BODY_LENGTH,
        .y = body_get_centroid(scene_get_body(state->body_list, 1)).y};
    body_set_centroid(scene_get_body(state->body_list, 1), new_pos);
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->body_list = scene_init();
  state->time = 0.0;
  state->counter = 0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  // first make the ball so that it is at index 0
  scene_add_body(
      state->body_list,
      make_ball(WINDOW_CENTER.x,
                WINDOW_MIN.y + 100)); // MAIN_BODY_HEIGHT + BALL_RADIUS + 5));
  // then  make the paddle so that it is at index 1
  scene_add_body(state->body_list,
                 make_main_body(WINDOW_CENTER.x, HIT_SIDE_POS_Y));
  // then make the two walls
  scene_add_body(state->body_list, make_vertical_wall(WINDOW_MIN.x, 250));
  scene_add_body(state->body_list,
                 make_vertical_wall(WINDOW_MAX.x + (WALL_WIDTH / 2), 250));
  scene_add_body(state->body_list,
                 make_horizontal_wall(500, WINDOW_MAX.y + (WALL_WIDTH / 2)));
  // then make all of the bricks so that they are at index 5-num_bricks
  spawn_bricks(state);
  /*for(size_t i = 0; i < 3; i ++){
    make_singular_row(state);
    if(i < 2){
      shift_all_rows(state);
    }
  }
  */
  ball_collision(state);
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    sdl_draw_polygon(body_get_vertices(scene_get_body(state->body_list, i)),
                     body_get_color(scene_get_body(state->body_list, i)));
  }
  // printf("%f\n", body_get_centroid(scene_get_body(state->body_list, 1)).x);
  // printf("%f\n", body_get_centroid(scene_get_body(state->body_list, 1)).y);
  // printf("%f\n", BALL_RADIUS);
  sdl_show();
  sdl_on_key(move_main_body);
  return state;
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time = state->time + dt;
  state->counter = state->counter + 1;
  if ((state->counter % 500) == 0) {
    shift_all_rows(state);
    make_singular_row(state);
  }
  check_hit_bottom(state);
  check_hit_walls(state);
  scene_tick(state->body_list, dt);
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    sdl_draw_polygon(body_get_vertices(scene_get_body(state->body_list, i)),
                     body_get_color(scene_get_body(state->body_list, i)));
  }
  sdl_show();
}

// frees the state
void emscripten_free(state_t *state) {
  scene_free(state->body_list);
  free(state);
}
