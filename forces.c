#include "forces.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "vector.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// initializes structs
typedef struct aux1 {
  body_t *body1;
  double constant;
} aux1_t;

typedef struct aux2 {
  body_t *body1;
  body_t *body2;
  double constant;
} aux2_t;

typedef struct aux_collide {
  body_t *body1;
  body_t *body2;
} aux_collide_t;

body_t *aux_generic_get_body1(aux_generic_t *aux) { return aux->body1; }

body_t *aux_generic_get_body2(aux_generic_t *aux) { return aux->body2; }

void *aux_generic_get_aux(aux_generic_t *aux) { return aux->aux; }

body_t *aux1_get_body1(aux1_t *aux1) { return aux1->body1; }

body_t *aux2_get_body1(aux2_t *aux2) { return aux2->body1; }

body_t *aux2_get_body2(aux2_t *aux2) { return aux2->body2; }

body_t *aux_collide_get_body1(aux_collide_t *aux) { return aux->body1; }

body_t *aux_collide_get_body2(aux_collide_t *aux) { return aux->body2; }

double aux1_get_constant(aux1_t *aux1) { return aux1->constant; }

double aux2_get_constant(aux2_t *aux2) { return aux2->constant; }

void aux_generic_set_unflagged(aux_generic_t *aux) { aux->flagged = false; }

collision_handler_t aux_generic_get_handler(aux_generic_t *aux) {
  return aux->handler;
}

bool aux_generic_get_flagged(aux_generic_t *aux) { return aux->flagged; }

void aux_generic_set_flagged(aux_generic_t *aux) { aux->flagged = true; }

void aux1_free(void *aux1) { free(aux1); }

void aux2_free(void *aux2) { free(aux2); }

void aux_collide_free(void *aux) { free(aux); }

void aux_generic_free(void *gen_aux) {
  if (((aux_generic_t *)gen_aux)->aux_freer != NULL) {
    ((aux_generic_t *)gen_aux)->aux_freer(((aux_generic_t *)gen_aux)->aux);
  }
  free(gen_aux);
}

// turns parameters into an auxiliary type and adds force creator to scene
void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux2_t *curr_aux = malloc(sizeof(aux2_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  curr_aux->constant = G;
  scene_add_bodies_force_creator(scene, (force_creator_t)(gravity_creator),
                                 curr_aux, bodies, aux2_free);
}

void create_normal(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux2_t *curr_aux = malloc(sizeof(aux2_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  curr_aux->constant = k;
  scene_add_bodies_force_creator(scene, (force_creator_t)normal_creator,
                                 curr_aux, bodies, aux2_free);
}

void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  list_add(bodies, body1);
  list_add(bodies, body2);
  aux2_t *curr_aux = malloc(sizeof(aux2_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  curr_aux->constant = k;
  scene_add_bodies_force_creator(scene, (force_creator_t)spring_creator,
                                 curr_aux, bodies, aux2_free);
}

void create_drag(scene_t *scene, double gamma, body_t *body) {
  list_t *bodies = list_init(1, (void *)body_free);
  list_add(bodies, body);
  aux1_t *curr_aux = malloc(sizeof(aux1_t));
  curr_aux->body1 = body;
  curr_aux->constant = gamma;
  scene_add_bodies_force_creator(scene, (force_creator_t)drag_creator, curr_aux,
                                 bodies, aux1_free);
}

void create_destructive_collision(scene_t *scene, body_t *body1,
                                  body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  aux_collide_t *curr_aux = malloc(sizeof(aux_collide_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  list_add(bodies, body1);
  list_add(bodies, body2);
  create_collision(scene, body1, body2, (collision_handler_t)collision_creator,
                   curr_aux, aux_collide_free);
}

void create_half_destructive_collision(scene_t *scene, double elasticity,
                                       body_t *body1, body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  aux2_t *curr_aux = malloc(sizeof(aux2_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  curr_aux->constant = elasticity;
  list_add(bodies, body1);
  list_add(bodies, body2);
  create_collision(scene, body1, body2,
                   (collision_handler_t)half_destructive_collision_creator,
                   curr_aux, aux2_free);
}

void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2) {
  list_t *bodies = list_init(2, (void *)body_free);
  aux2_t *curr_aux = malloc(sizeof(aux2_t));
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  curr_aux->constant = elasticity;
  list_add(bodies, body1);
  list_add(bodies, body2);
  create_collision(scene, body1, body2,
                   (collision_handler_t)physics_collision_creator, curr_aux,
                   aux2_free);
}

void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer) {
  list_t *bodies = list_init(2, (void *)body_free);
  aux_generic_t *curr_aux = malloc(sizeof(aux_generic_t));
  curr_aux->aux = aux;
  curr_aux->flagged = false;
  curr_aux->handler = handler;
  curr_aux->body1 = body1;
  curr_aux->body2 = body2;
  list_add(bodies, body1);
  list_add(bodies, body2);
  curr_aux->aux_freer = freer;
  scene_add_bodies_force_creator(scene,
                                 (force_creator_t)generic_collision_creator,
                                 curr_aux, bodies, aux_generic_free);
}
