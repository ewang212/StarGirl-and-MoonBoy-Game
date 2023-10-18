#include "polygon.h"
#include "color.h"
#include "list.h"
#include "sdl_wrapper.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const vector_t CENTER_P = {.x = 500, .y = 250};

// star constants
const size_t NUM_CIRC_POINTS_P = 10;
const double STAR_RAD_P = 100;
const double SPEED_P = 30.0;
const double ROTATE_P = .05;

const double GRAVITY = 10;
const double ELASTICITY = .98;

// color constants
const float YELLOW_R_P = 1.0;
const float YELLOW_G_P = 0.5;
const float YELLOW_B_P = 0.5;

const double TWO_PI_P = 2.0 * M_PI;

typedef struct polygon {
  list_t *vertices;
  vector_t *velocity;
  rgb_color_t color;
} polygon_t;

// initializes polygon characteristics
polygon_t *polygon_init(list_t *vertices) {
  polygon_t *new_polygon = malloc(sizeof(polygon_t));
  assert(new_polygon != NULL);

  new_polygon->vertices = vertices;

  vector_t *velocity = malloc(sizeof(vector_t));
  velocity->x = SPEED_P;
  velocity->y = 0;
  new_polygon->velocity = velocity;

  // randomly generates colors
  rgb_color_t new_color = {.r = (float)rand() / (float)RAND_MAX,
                           .g = (float)rand() / (float)RAND_MAX,
                           .b = (float)rand() / (float)RAND_MAX};
  new_polygon->color = new_color;
  return new_polygon;
}

list_t *polygon_get_vertices(polygon_t *polygon) { return polygon->vertices; }

// returning list of vertices of star (counter-clockwise)
list_t *polygon_make_star_p(size_t radius, size_t num_points, size_t center_x,
                            size_t center_y) {
  double x;
  double y;
  list_t *vertices = list_init(num_points, free);

  for (size_t i = 0; i <= num_points - 1; i += 2) {

    // outside points
    x = radius * cos((TWO_PI_P * (i / 2)) / (num_points / 2)) + center_x;
    y = radius * sin((TWO_PI_P * (i / 2)) / (num_points / 2)) + center_y;

    vector_t *temp = malloc(sizeof(vector_t));
    *temp = (vector_t){x, y};
    list_add(vertices, temp);

    // inside points
    x = (radius / 2) * cos(TWO_PI_P * (i + 1) / num_points) + center_x;
    y = (radius / 2) * sin(TWO_PI_P * (i + 1) / num_points) + center_y;

    vector_t *tem = malloc(sizeof(vector_t));
    *tem = (vector_t){x, y};
    list_add(vertices, tem);
  }
  return vertices;
}

double polygon_area(list_t *polygon) {
  // using shoelace formula
  double area = 0.0;
  size_t len = list_size(polygon);
  for (size_t i = 0; i < len; i++) {
    area += vec_cross(*((vector_t *)list_get(polygon, i % len)),
                      *((vector_t *)list_get(polygon, (i + 1) % len)));
  }
  area = area / 2.0;

  return area;
}

vector_t polygon_centroid(list_t *polygon) {
  // using centroid formula for x and y individually
  double x = 0.0;
  double y = 0.0;
  size_t len = list_size(polygon);
  for (size_t i = 0; i < len; i++) {
    vector_t vec_1 = (*(vector_t *)list_get(polygon, i % len));
    vector_t vec_2 = (*(vector_t *)list_get(polygon, (i + 1) % len));
    double x1 = vec_1.x;
    double y1 = vec_1.y;
    double x2 = vec_2.x;
    double y2 = vec_2.y;

    x += ((x1 + x2) * vec_cross(vec_1, vec_2));
    y += ((y1 + y2) * vec_cross(vec_1, vec_2));
  }

  x = (1 / (6 * polygon_area(polygon))) * x;
  y = (1 / (6 * polygon_area(polygon))) * y;

  vector_t centroid = {.x = x, .y = y};
  return centroid;
}

void polygon_translate(list_t *polygon, vector_t translation) {
  // adding translation vector to each vector in polygon
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t temp = vec_add((*(vector_t *)list_get(polygon, i)), translation);
    *((vector_t *)list_get(polygon, i)) = temp;
  }
}

void polygon_rotate(list_t *polygon, double angle, vector_t point) {
  // subtracting point, rotating wrt to (0,0), adding point back
  for (size_t i = 0; i < list_size(polygon); i++) {
    vector_t sub = vec_subtract((*(vector_t *)list_get(polygon, i)), point);
    vector_t rot = vec_rotate(sub, angle);
    vector_t add = vec_add(rot, point);
    *((vector_t *)list_get(polygon, i)) = add;
  }
}

// adjusts position after hitting bottom
void polygon_hitting_bottom(polygon_t *polygon, list_t *vertices) {
  for (size_t i = 0; i < list_size(vertices); i++) {
    if ((*(vector_t *)list_get(vertices, i)).y < 0) {
      // apply momentum - change sign of y velocity
      polygon->velocity->y = polygon->velocity->y * -1 * ELASTICITY;
      vector_t dist = {.x = 0, .y = 0 - (*(vector_t *)list_get(vertices, i)).y};
      polygon_translate(polygon->vertices, dist);
    }
  }
}

// translates and rotates the star
void polygon_star_movement(polygon_t *polygon, double time) {
  list_t *vertices = polygon->vertices;

  // update velocity
  double old_velocity = polygon->velocity->y;
  double new_velocity = old_velocity - (GRAVITY * time);
  polygon->velocity->y = new_velocity;

  vector_t trans = {.x = polygon->velocity->x * time,
                    .y = polygon->velocity->y * time};
  polygon_translate(vertices, trans);
  vector_t rot = polygon_centroid(vertices);
  polygon_rotate(vertices, ROTATE_P, rot);
  polygon_hitting_bottom(polygon, vertices); // checks boundary domains
}

void polygon_free(polygon_t *polygon) {
  list_t *vertices = polygon->vertices;
  vector_t *velocity = polygon->velocity;
  list_free(vertices);
  free(velocity);
}

rgb_color_t polygon_get_color(polygon_t *polygon) { return polygon->color; }
