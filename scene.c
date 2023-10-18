#include "body.h"
#include "collision.h"
#include "forces.h"
#include "list.h"
#include "vector.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const size_t START_BODIES = 30;
const double MIN_DIST = 5.0;
const double RESET_DIST = 30.0;

typedef struct force {
  force_creator_t forcer;
  void *aux;
  free_func_t freer;
  list_t *bodies;
  // collision_info_t collision_info; // where do we use this
} force_t;

typedef struct scene {
  list_t *bodies;
  list_t *forces;
} scene_t;

scene_t *scene_init(void) {
  scene_t *empty_scene = malloc(sizeof(scene_t));
  assert(empty_scene != NULL);

  list_t *empty_bodies = list_init(START_BODIES, (void *)body_free);
  assert(empty_bodies != NULL);

  list_t *empty_forces = list_init(1, (void *)force_free);
  assert(empty_forces != NULL);

  empty_scene->bodies = empty_bodies;
  empty_scene->forces = empty_forces;
  return empty_scene;
}

void scene_free(scene_t *scene) {
  list_free(scene->bodies);
  list_free(scene->forces);
  free(scene);
}

void force_free(force_t *force) {
  if (force->freer != NULL) {
    force->freer(force->aux);
  }
  for (size_t i = 0; i < list_size(force->bodies); i++) {
    list_remove(force->bodies, i);
    i--;
  }
  list_free(force->bodies);
  free(force);
}

void gravity_creator(void *aux) {
  // calculates distance between bodies
  double x_sub = powf(body_get_centroid(aux2_get_body2(aux)).x -
                          body_get_centroid(aux2_get_body1(aux)).x,
                      2);
  double y_sub = powf(body_get_centroid(aux2_get_body2(aux)).y -
                          body_get_centroid(aux2_get_body1(aux)).y,
                      2);
  double dist_squared = x_sub + y_sub;
  double dist = sqrt(dist_squared);
  if (dist < MIN_DIST) {
    dist = RESET_DIST;
  }

  // calculates force magnitude with F=GmM/r^2
  double force_mag =
      (aux2_get_constant(aux) * (body_get_mass(aux2_get_body1(aux))) *
       (body_get_mass(aux2_get_body2(aux)))) /
      dist_squared;

  // finds direction of force of body 2 on body 1
  vector_t r_21 = vec_multiply(
      1.0 / dist, vec_subtract(body_get_centroid(aux2_get_body2(aux)),
                               body_get_centroid(aux2_get_body1(aux))));
  vector_t force_21 = vec_multiply(force_mag, r_21);
  vector_t force_12 = vec_multiply(-1.0, force_21);
  // if flag for removal is not true then add force
  if (body_get_grav(aux2_get_body2(aux)) == 0) {
    // body_add_force(aux2_get_body1(aux), force_21);
    body_add_force(aux2_get_body2(aux), force_12);
  }
}

void normal_creator(void *aux) {
  // double magnitude =
  //  body_get_mass(aux2_get_body1(aux)) * aux2_get_constant(aux);
  // vector_t direction = (vector_t){.x = 0, .y = -1};
  // vector_t force1 = vec_multiply(magnitude, direction);
  // body_add_force(aux2_get_body2(aux), force1);
  // body1 is platform
  /* if(body_get_force_removal(aux2_get_body1(aux)) == 1){
     body_remove_force(aux2_get_body2(aux), force1);
   }
   if(body_get_force_removal(aux2_get_body1(aux)) == 2) {
     body_add_force(aux2_get_body2(aux), force1);
   }
   */
}

void spring_creator(void *aux) {
  // finds distance
  double x_sub = powf(body_get_centroid(aux2_get_body2(aux)).x -
                          body_get_centroid(aux2_get_body1(aux)).x,
                      2);
  double y_sub = powf(body_get_centroid(aux2_get_body2(aux)).y -
                          body_get_centroid(aux2_get_body1(aux)).y,
                      2);
  double dist_squared = x_sub + y_sub;
  double dist = sqrt(dist_squared);

  // calculates unit vector and forces, adds forces to body
  vector_t r_21 = vec_multiply(
      1.0 / dist, vec_subtract(body_get_centroid(aux2_get_body2(aux)),
                               body_get_centroid(aux2_get_body1(aux))));
  vector_t force1 = vec_multiply(aux2_get_constant(aux) * dist, r_21);
  vector_t force2 = vec_multiply((-1.0), force1);

  body_add_force(aux2_get_body1(aux), force1);
  body_add_force(aux2_get_body2(aux), force2);
}

void physics_collision_creator(body_t *body1, body_t *body2, vector_t axis,
                               void *aux) {
  // finds impulse using aux2
  double body1_mass = body_get_mass(body1);
  double body2_mass = body_get_mass(body2);
  double body1_ua = vec_dot(body_get_velocity(body1), axis);
  double body2_ub = vec_dot(body_get_velocity(body2), axis);
  double elasticity = aux2_get_constant(aux);

  double impulse_mag = (1 + elasticity) * (body2_ub - body1_ua);

  // edge cases based on if mass is infinity
  if (body1_mass == INFINITY) {
    impulse_mag *= body2_mass;
  } else if (body2_mass == INFINITY) {
    impulse_mag *= body1_mass;
  } else if (body1_mass == INFINITY && body2_mass == INFINITY) {
    impulse_mag = 0;
  } else {
    impulse_mag *= ((body1_mass * body2_mass) / (body1_mass + body2_mass));
  }

  vector_t impulse_21 = vec_multiply(impulse_mag, axis);
  vector_t impulse_12 = vec_multiply(-1, impulse_21);

  //  body_add_impulse(body1, impulse_21);
  //  body_add_impulse(body2, impulse_12);
  body_add_impulse(body2, impulse_12);
}

// applies collision handler
void generic_collision_creator(void *aux) {
  collision_info_t info = find_collision(
      body_get_vertices(aux_generic_get_body1((aux_generic_t *)aux)),
      body_get_vertices(aux_generic_get_body2((aux_generic_t *)aux)));
  if (info.collided == 1 && aux_generic_get_flagged(aux) == false) {
    aux_generic_set_flagged((aux_generic_t *)aux);
    aux_generic_get_handler((aux_generic_t *)aux)(
        aux_generic_get_body1((aux_generic_t *)aux),
        aux_generic_get_body2((aux_generic_t *)aux), info.axis,
        aux_generic_get_aux(aux));
  } else if (info.collided == 0) {
    aux_generic_set_unflagged((aux_generic_t *)aux);
  }
}

void drag_creator(void *aux) {
  // uses drag = - gamma * velocity to find drag force
  vector_t force = vec_multiply((-1) * aux1_get_constant(aux),
                                body_get_velocity(aux1_get_body1(aux)));
  body_add_force(aux1_get_body1(aux), force);
}

void collision_creator(body_t *body1, body_t *body2, vector_t axis, void *aux) {
  body_remove(body1);
  body_remove(body2);
}

// creates collision where only one body is removed
void half_destructive_collision_creator(body_t *body1, body_t *body2,
                                        vector_t axis, void *aux) {
  // calculates impulse
  double body1_mass = body_get_mass(body1);
  double body2_mass = body_get_mass(body2);
  double body1_ua = vec_dot(body_get_velocity(body1), axis);
  double body2_ub = vec_dot(body_get_velocity(body2), axis);
  double elasticity = aux2_get_constant(aux);

  double impulse_mag = (1 + elasticity) * (body2_ub - body1_ua);

  if (body1_mass == INFINITY) {
    impulse_mag *= body2_mass;
  } else if (body2_mass == INFINITY) {
    impulse_mag *= body1_mass;
  } else {
    impulse_mag *= ((body1_mass * body2_mass) / (body1_mass + body2_mass));
  }

  vector_t impulse_21 = vec_multiply(impulse_mag, axis);
  vector_t impulse_12 = vec_multiply(-1.0, impulse_21);

  // applies impulse to one body, removes other
  body_remove(body1);
  body_add_impulse(body2, impulse_12);
}

size_t scene_bodies(scene_t *scene) { return list_size(scene->bodies); }

size_t scene_forces(scene_t *scene) { return list_size(scene->forces); }

body_t *scene_get_body(scene_t *scene, size_t index) {
  assert(index < scene_bodies(scene) && index >= 0);
  return (body_t *)list_get(scene->bodies, index);
}

void scene_add_body(scene_t *scene, body_t *body) {
  list_add(scene->bodies, body);
}

void scene_remove_body_fake(scene_t *scene, size_t index) {
  list_remove(scene->bodies, index);
}

void scene_remove_body(scene_t *scene, size_t index) {
  assert(index < scene_bodies(scene) && index >= 0);
  void *old_body = list_get(scene->bodies, index);
  body_remove(old_body);
}

void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer) {
  force_t *force = malloc(sizeof(force_t));
  force->forcer = forcer;
  force->aux = aux;
  force->freer = freer;
  force->bodies = bodies;
  list_add(scene->forces, force);
}

void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer) {
  list_t *bodies = list_init(0, (void *)body_free);
  scene_add_bodies_force_creator(scene, forcer, aux, bodies, freer);
}

void scene_tick(scene_t *scene, double dt) {
  // calls forces using appropriate forcer
  for (size_t i = 0; i < list_size(scene->forces); i++) {
    force_t *creator = ((force_t *)list_get(scene->forces, i));
    assert(creator->forcer != NULL);
    assert(creator->aux != NULL);
    creator->forcer(creator->aux);
  }
  for (size_t i = 0; i < list_size(scene->bodies); i++) {
    body_tick(list_get(scene->bodies, i), dt);
  }
  // checks for removal
  for (size_t i = 0; i < scene_bodies(scene); i++) {
    body_t *curr_body = (body_t *)(list_get(scene->bodies, i));
    // if body is flagged for removal
    if (body_is_removed(curr_body)) {
      // loop through forces
      for (size_t j = 0; j < scene_forces(scene); j++) {
        force_t *curr_force = (force_t *)(list_get(scene->forces, j));
        list_t *body_list = curr_force->bodies;
        // loop through bodies list for current force
        for (size_t k = 0; k < list_size(body_list); k++) {
          // check if the current body is in the current force's bodies list
          if ((body_t *)(list_get(body_list, k)) == curr_body) {
            // if it is, remove & free the force from the scene
            force_t *tem = (force_t *)(list_remove(scene->forces, j));
            force_free(tem);
            j--;
            break;
          }
        }
      }
      body_t *temp = (body_t *)list_remove(scene->bodies, i);
      body_free(temp);
      i--;
    }
  }
}
