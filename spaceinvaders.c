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
const size_t NUM_ROWS = 3;
const size_t NUM_COLUMNS = 14;
// miscellaneous constants
const float PI = M_PI;
const size_t HALF_CIRCLE = 180;
const size_t CHAR_MALLOC = 17;
const size_t SPAWN_PROJ_OR_SHIP = 100;
// main body constants
const double MAIN_RADIUS = 10;
const double MAIN_MASS = 20;
const double MAIN_POINTS = 360;
const double MAIN_ACCELERATION = 10;
const vector_t MAIN_VELOCITY = {.x = 100, .y = 0};
// invader constants
const double INVADER_RADIUS_X = 35;
const double INVADER_RADIUS_Y = 25;
const double INVADER_MASS = 10.0;
const size_t NUM_INIT_INVADERS = 30;
const size_t INVADER_CIRC_POINTS = 181;
const size_t INVADER_NONCIRC_POINTS = 125;
const size_t INVADER_INCREMENT = 15;
const size_t INVADER_ANGLE_INCREMENT = 29;
const size_t INVADER_POS_INCREMENT = 10;
const double INVADER_ACCELERATION = 10;
const vector_t INVADER_INIT_LOC = {.x = 950, .y = 450};
const vector_t INVADER_VELOCITY = {.x = 75, .y = 0};
const rgb_color_t INVADER_COLOR = {0, 0.9, 1.0};
// projectile constants
const double PROJECTILE_LENGTH = 8;
const double PROJECTILE_HEIGHT = 15;
const double PROJECTILE_MASS = 2;
const double PROJECTILE_ACCELERATION = 30;
const vector_t PROJECTILE_VELOCITY = {.x = 0, .y = 60};
// main body constants
const double MAIN_BODY_LENGTH = 40;
const double MAIN_BODY_HEIGHT = 18;
const double MAIN_BODY_MASS = 8;
const rgb_color_t MAIN_BODY_COLOR = {1.0, 0.6, 0.9};
// hitting side constants
const size_t HIT_SIDE_POS_Y = 100;
const size_t HIT_SIDE_INVADER = 700;

typedef struct state {
  scene_t *body_list;
  double time;
  size_t counter;
} state_t;

// this function makes the shape of the pellets
body_t *make_rectangle(size_t center_x, size_t center_y, char *version,
                       rgb_color_t color) {
  // make list & add vertices of rectangle
  list_t *vertices = list_init(4, free);
  assert(vertices != NULL);

  vector_t *pt1 = malloc(sizeof(vector_t));
  *pt1 = (vector_t){.x = center_x + (PROJECTILE_LENGTH / 2),
                    .y = center_y + (PROJECTILE_HEIGHT / 2)};
  list_add(vertices, pt1);

  vector_t *pt2 = malloc(sizeof(vector_t));
  *pt2 = (vector_t){.x = center_x - (PROJECTILE_LENGTH / 2),
                    .y = center_y + (PROJECTILE_HEIGHT / 2)};
  list_add(vertices, pt2);

  vector_t *pt3 = malloc(sizeof(vector_t));
  *pt3 = (vector_t){.x = center_x - (PROJECTILE_LENGTH / 2),
                    .y = center_y - (PROJECTILE_HEIGHT / 2)};
  list_add(vertices, pt3);

  vector_t *pt4 = malloc(sizeof(vector_t));
  *pt4 = (vector_t){.x = center_x + (PROJECTILE_LENGTH / 2),
                    .y = center_y - (PROJECTILE_HEIGHT / 2)};
  list_add(vertices, pt4);

  body_t *rectangle =
      body_init_with_info(vertices, PROJECTILE_MASS, color, version, free);

  // add center point to verticies
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(rectangle, center);
  return rectangle;
}

// This function makes the shape for the space invaders
body_t *make_ships(size_t center_x, size_t center_y) {
  char *type = malloc(sizeof(char) * INVADER_INCREMENT);
  strcpy(type, "space invader");

  size_t num_points = INVADER_CIRC_POINTS;
  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);

  for (size_t i = 1; i < INVADER_NONCIRC_POINTS; i = i + INVADER_INCREMENT) {
    double curr_angle = ((i + INVADER_ANGLE_INCREMENT) * PI) / HALF_CIRCLE;
    vector_t *pt = malloc(sizeof(vector_t));
    *pt = (vector_t){.x = cos(curr_angle) * INVADER_RADIUS_X + center_x,
                     .y = sin(curr_angle) * INVADER_RADIUS_Y + center_y};
    list_add(vertices, pt);
  }

  vector_t *bottom_pt = malloc(sizeof(vector_t));
  *bottom_pt =
      (vector_t){.x = center_x, .y = center_y - (INVADER_RADIUS_Y / 4)};
  list_add(vertices, bottom_pt);

  body_t *ship =
      body_init_with_info(vertices, INVADER_MASS, INVADER_COLOR, type, free);

  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(ship, center);
  body_set_velocity(ship, INVADER_VELOCITY);
  return ship;
}

// this function makes the shape for the user/main body
body_t *make_oval(size_t center_x, size_t center_y) {
  char *type = malloc(sizeof(char) * 11);
  strcpy(type, "main_body");

  size_t num_points = MAIN_POINTS;
  list_t *vertices = list_init(num_points, free);
  assert(vertices != NULL);

  for (double i = MAIN_BODY_LENGTH; i > ((-1) * MAIN_BODY_LENGTH); i--) {
    vector_t *pos_pt = malloc(sizeof(vector_t));
    double x = i + center_x;
    double y =
        sqrt(pow(MAIN_BODY_HEIGHT, 2) *
             (1 - ((pow((x - center_x), 2)) / pow(MAIN_BODY_LENGTH, 2)))) +
        center_y;
    *pos_pt = (vector_t){.x = x, .y = y};
    list_add(vertices, pos_pt);
  }

  list_t *top_vertices = list_init(list_size(vertices), free);
  for (size_t i = 0; i < list_size(vertices); i++) {
    vector_t *vertex = malloc(sizeof(vector_t));
    vector_t *curr = (vector_t *)list_get(vertices, i);
    vertex->x = curr->x;
    vertex->y = curr->y;
    list_add(top_vertices, vertex);
  }

  for (double i = list_size(top_vertices) - 1; i >= 0; i--) {
    vector_t *neg_pt = malloc(sizeof(vector_t));
    double x = ((vector_t *)list_get(top_vertices, i))->x;
    double diff = ((vector_t *)list_get(top_vertices, i))->y - center_y;
    double y = ((vector_t *)list_get(top_vertices, i))->y - (2 * diff);
    *neg_pt = (vector_t){.x = x, .y = y};
    list_add(vertices, neg_pt);
  }

  free(top_vertices);

  body_t *oval = body_init_with_info(vertices, MAIN_BODY_MASS, MAIN_BODY_COLOR,
                                     type, (void *)free);

  // add center point to vertices
  vector_t center = (vector_t){.x = center_x, .y = center_y};
  body_set_centroid(oval, center);

  return oval;
}

// This function tests if the user hits the left or right side of the window
void hitting_sides(state_t *state) {
  body_t *user = scene_get_body(state->body_list, 0);
  double min_centroid = WINDOW_BODY_MAX.x - MAIN_RADIUS - 2;
  if (body_get_centroid(user).x + MAIN_RADIUS > WINDOW_BODY_MAX.x) {
    body_set_centroid(user, (vector_t){.x = min_centroid, .y = HIT_SIDE_POS_Y});
  }
  double max_centroid = WINDOW_BODY_MIN.x + MAIN_RADIUS + 2;
  if (body_get_centroid(user).x - MAIN_RADIUS < WINDOW_BODY_MIN.x) {
    body_set_centroid(user, (vector_t){.x = max_centroid, .y = HIT_SIDE_POS_Y});
  }
}

// This function makes the user projectile
void make_user_projectile(state_t *state) {
  body_t *user = scene_get_body(state->body_list, 0);
  vector_t curr = body_get_centroid(user);
  char *type = malloc(sizeof(char) * CHAR_MALLOC);
  strcpy(type, "user projectile");
  body_t *proj = make_rectangle(curr.x, curr.y + INVADER_POS_INCREMENT, type,
                                MAIN_BODY_COLOR);
  scene_add_body(state->body_list, proj);
  body_set_velocity(proj, PROJECTILE_VELOCITY);

  for (size_t i = 1; i < scene_bodies(state->body_list); i++) {
    assert(scene_get_body(state->body_list, i) != NULL);
    if (strcmp((((char *)body_get_info(scene_get_body(state->body_list, i)))),
               "space invader") == 0) {
      create_destructive_collision(state->body_list, proj,
                                   scene_get_body(state->body_list, i));
    }
  }
}

// This function makes the enemy projectile
void make_enemy_projectile(state_t *state) {
  size_t ran = rand() % (scene_bodies(state->body_list));
  while (
      strcmp((((char *)body_get_info(scene_get_body(state->body_list, ran)))),
             "space invader") != 0) {
    ran = rand() % (scene_bodies(state->body_list));
  }
  body_t *ran_body = scene_get_body(state->body_list, ran);
  vector_t curr = body_get_centroid(ran_body);
  char *type = malloc(sizeof(char) * (CHAR_MALLOC + 1));
  strcpy(type, "enemy projectile");
  body_t *proj = make_rectangle(curr.x, curr.y, type, INVADER_COLOR);
  scene_add_body(state->body_list, proj);
  body_set_velocity(proj, vec_multiply(-1, PROJECTILE_VELOCITY));
  create_destructive_collision(state->body_list, proj,
                               scene_get_body(state->body_list, 0));
  if (strcmp((((char *)body_get_info(scene_get_body(state->body_list, 0)))),
             "main_body") != 0) {
    printf("%s\n", "Game over loser.");
    exit(0);
  }
}

// moves user on key press
void move_main_body(char key, key_event_type_t pressed, double time_held,
                    state_t *state) {
  body_t *user = scene_get_body(state->body_list, 0);
  if (pressed == KEY_PRESSED) {
    if (key == LEFT_ARROW) {
      body_set_velocity(user, vec_multiply(-1, MAIN_VELOCITY));
      hitting_sides(state);
    } else if (key == RIGHT_ARROW) {
      body_set_velocity(user, MAIN_VELOCITY);
      hitting_sides(state);
    } else if (key == ' ') {
      make_user_projectile(state);
    }
  } else {
    body_set_velocity(user, VEC_ZERO);
  }
}

// moves space invader
void move_space_invader(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    if (strcmp(((char *)body_get_info(scene_get_body(state->body_list, i))),
               "space invader") == 0) {
      body_t *space_invader = scene_get_body(state->body_list, i);
      if (body_get_centroid(space_invader).x + INVADER_RADIUS_X >=
              WINDOW_MAX.x - INVADER_POS_INCREMENT ||
          body_get_centroid(space_invader).x - INVADER_RADIUS_X <=
              WINDOW_MIN.x + INVADER_POS_INCREMENT) {
        double left_right;
        if (body_get_centroid(space_invader).x > HIT_SIDE_INVADER) {
          left_right = WINDOW_BODY_MAX.x;
        } else {
          left_right = WINDOW_BODY_MIN.x;
        }
        vector_t update =
            (vector_t){.x = left_right,
                       .y = body_get_centroid(space_invader).y -
                            2 * (INVADER_RADIUS_Y * 2) - INVADER_POS_INCREMENT};
        body_set_centroid(space_invader, update);
        body_set_velocity(space_invader,
                          vec_multiply(-1, body_get_velocity(space_invader)));
      }
      if (body_get_centroid(space_invader).y - INVADER_RADIUS_Y <=
          WINDOW_MIN.y) {
        printf("%s\n", "Space invader collision with window min, END GAME");
        exit(0);
      }
    }
  }
}

// sets the space invaders in three rows
void spawn_ships(state_t *state) {
  double x = WINDOW_MIN.x + WINDOW_BODY_MIN.x;
  double y = WINDOW_MAX.y - WINDOW_BODY_MIN.x;
  for (int i = 0; i < NUM_COLUMNS; i++) {
    scene_add_body(state->body_list, make_ships(x - INVADER_RADIUS_X * i, y));
    scene_add_body(state->body_list,
                   make_ships(x - INVADER_RADIUS_X * i,
                              y - INVADER_RADIUS_Y - INVADER_POS_INCREMENT));
    scene_add_body(
        state->body_list,
        make_ships(x - INVADER_RADIUS_X * i,
                   y - (INVADER_RADIUS_Y + INVADER_POS_INCREMENT) * 2));
    x += SPAWN_PROJ_OR_SHIP;
  }
}

state_t *emscripten_init() {
  state_t *state = malloc(sizeof(state_t));
  state->body_list = scene_init();
  state->counter = 0;
  state->time = 0.0;
  sdl_init(WINDOW_MIN, WINDOW_MAX);
  scene_add_body(state->body_list, make_oval(500, 100));
  spawn_ships(state);
  make_user_projectile(state);
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    sdl_draw_polygon(body_get_vertices(scene_get_body(state->body_list, i)),
                     body_get_color(scene_get_body(state->body_list, i)));
  }
  sdl_show();
  sdl_on_key(move_main_body);
  return state;
}

// removes projectile off screen
void check_projectile(state_t *state) {
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    if (strcmp((((char *)body_get_info(scene_get_body(state->body_list, i)))),
               "enemy projectile") == 0 &&
        body_get_centroid(scene_get_body(state->body_list, i)).y <
            (WINDOW_MIN.y - PROJECTILE_HEIGHT)) {
      body_remove(scene_get_body(state->body_list, i));
    }
    if ((strcmp((((char *)body_get_info(scene_get_body(state->body_list, i)))),
                "user projectile") == 0) &&
        body_get_centroid(scene_get_body(state->body_list, i)).y >
            (WINDOW_MAX.y + PROJECTILE_HEIGHT)) {
      body_remove(scene_get_body(state->body_list, i));
    }
  }
}

// checks if the user is hit by an enemy projectile
void main_body_projectile_collision(state_t *state) {
  for (size_t i = 1; i < scene_bodies(state->body_list); i++) {
    if (strcmp((((char *)body_get_info(scene_get_body(state->body_list, i)))),
               "enemy projectile") == 0) {
      if (find_collision(body_get_vertices(scene_get_body(state->body_list, 0)),
                         body_get_vertices(scene_get_body(state->body_list, i)))
              .collided == 1) {
        printf("%s\n",
               "Collision with main body and enemy projectile. You lose!");
        body_remove(scene_get_body(state->body_list, 0));
        body_remove(scene_get_body(state->body_list, i));
        exit(0);
      }
    }
  }
}

// ends game once all space invaders are hit by user projectiles
void clear_space_invaders(state_t *state) {
  bool flag = true;
  for (size_t i = 0; i < scene_bodies(state->body_list); i++) {
    if (strcmp((((char *)body_get_info(scene_get_body(state->body_list, i)))),
               "space invader") == 0) {
      flag = false;
      break;
    }
  }
  if (flag == true) {
    printf("%s\n", "You cleared all the space invaders! Hooray!");
    exit(0);
  }
}

void emscripten_main(state_t *state) {
  sdl_clear();
  double dt = time_since_last_tick();
  state->time = state->time + dt;
  state->counter = state->counter + 1;
  move_space_invader(state);
  if (state->counter % SPAWN_PROJ_OR_SHIP == 0) {
    make_enemy_projectile(state);
  }
  check_projectile(state);
  hitting_sides(state);

  main_body_projectile_collision(state);
  clear_space_invaders(state);
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
