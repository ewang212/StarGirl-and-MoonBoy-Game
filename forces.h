#ifndef __FORCES_H__
#define __FORCES_H__

#include "scene.h"

typedef struct aux1 aux1_t;
typedef struct aux2 aux2_t;
typedef struct aux_collide aux_collide_t;

typedef struct aux_generic {
  void *aux;
  body_t *body1;
  body_t *body2;
  bool flagged;
  collision_handler_t handler;
  free_func_t aux_freer;
} aux_generic_t;

/**
 * Frees aux1
 *
 * @param aux1 a pointer to aux1
 */
void aux1_free(void *aux1);

/**
 * Frees aux2
 *
 * @param aux2 a pointer to aux2
 */
void aux2_free(void *aux2);

/**
 * Frees aux collide
 *
 * @param aux a pointer to aux
 */
void aux_collide_free(void *aux);

/**
 * Gets the first body in aux1
 *
 * @param aux1 a pointer to aux1
 */
body_t *aux1_get_body1(aux1_t *aux1);

/**
 * Gets the first body in aux2
 *
 * @param aux2 a pointer to aux2
 */
body_t *aux2_get_body1(aux2_t *aux2);

/**
 * Gets the second body in aux2
 *
 * @param aux2 a pointer to aux2
 */
body_t *aux2_get_body2(aux2_t *aux2);

/**
 * Gets the first body in aux collide
 *
 * @param aux a pointer to aux
 */
body_t *aux_collide_get_body1(aux_collide_t *aux);

/**
 * Gets the second body in aux collide
 *
 * @param aux a pointer to aux
 */
body_t *aux_collide_get_body2(aux_collide_t *aux);

/**
 * Gets the first body in aux generic
 *
 * @param aux a pointer to aux
 */
body_t *aux_generic_get_body1(aux_generic_t *aux);

/**
 * Gets the second body in aux generic
 *
 * @param aux a pointer to aux
 */
body_t *aux_generic_get_body2(aux_generic_t *aux);

/**
 * Gets flag in aux generic
 *
 * @param aux a pointer to aux
 */
bool aux_generic_get_flagged(aux_generic_t *aux);

/**
 * Sets flag in aux generic
 *
 * @param aux a pointer to aux
 */
void aux_generic_set_flagged(aux_generic_t *aux);

/**
 * Gets flag in aux2
 *
 * @param aux a pointer to aux
 */
bool aux2_get_flagged(aux2_t *aux);

/**
 * Sets flag in aux2
 *
 * @param aux a pointer to aux
 */
void aux2_set_flagged(aux2_t *aux);

/**
 * Gets aux generic
 *
 * @param aux a pointer to aux
 */
void *aux_generic_get_aux(aux_generic_t *aux);

/**
 * Gets the constant in aux1
 *
 * @param aux1 a pointer to aux1
 */
double aux1_get_constant(aux1_t *aux1);

/**
 * Gets the constant in aux2
 *
 * @param aux2 a pointer to aux2
 */
double aux2_get_constant(aux2_t *aux2);

/**
 * Sets unflag in aux generic
 *
 * @param aux1 a pointer to aux
 */
void aux_generic_set_unflagged(aux_generic_t *aux);

/**
 * Sets unflag in aux2
 *
 * @param aux2 a pointer to aux
 */
void aux2_set_unflagged(aux2_t *aux);

/**
 * Returns the handler
 */
collision_handler_t aux_generic_get_handler(aux_generic_t *aux);

/**
 * Adds a force creator to a scene that applies gravity between two bodies.
 * The force creator will be called each tick
 * to compute the Newtonian gravitational force between the bodies.
 * See
 * https://en.wikipedia.org/wiki/Newton%27s_law_of_universal_gravitation#Vector_form.
 * The force should not be applied when the bodies are very close,
 * because its magnitude blows up as the distance between the bodies goes to 0.
 *
 * @param scene the scene containing the bodies
 * @param G the gravitational proportionality constant
 * @param body1 the first body
 * @param body2 the second body
 */
void create_newtonian_gravity(scene_t *scene, double G, body_t *body1,
                              body_t *body2);

void create_normal(scene_t *scene, double k, body_t *body1, body_t *body2);
/**
 * Adds a force creator to a scene that acts like a spring between two bodies.
 * The force creator will be called each tick
 * to compute the Hooke's-Law spring force between the bodies.
 * See https://en.wikipedia.org/wiki/Hooke%27s_law.
 *
 * @param scene the scene containing the bodies
 * @param k the Hooke's constant for the spring
 * @param body1 the first body
 * @param body2 the second body
 */
void create_spring(scene_t *scene, double k, body_t *body1, body_t *body2);

/**
 * Adds a force creator to a scene that applies a drag force on a body.
 * The force creator will be called each tick
 * to compute the drag force on the body proportional to its velocity.
 * The force points opposite the body's velocity.
 *
 * @param scene the scene containing the bodies
 * @param gamma the proportionality constant between force and velocity
 *   (higher gamma means more drag)
 * @param body the body to slow down
 */
void create_drag(scene_t *scene, double gamma, body_t *body);

/**
 * Adds a force creator to a scene that calls a given collision handler
 * function each time two bodies collide.
 * This generalizes create_destructive_collision() from last week,
 * allowing different things to happen on a collision.
 * The handler is passed the bodies, the collision axis, and an auxiliary value.
 * It should only be called once while the bodies are still colliding.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 * @param handler a function to call whenever the bodies collide
 * @param aux an auxiliary value to pass to the handler
 * @param freer if non-NULL, a function to call in order to free aux
 */
void create_collision(scene_t *scene, body_t *body1, body_t *body2,
                      collision_handler_t handler, void *aux,
                      free_func_t freer);

/**
 * Adds a force creator to a scene that destroys two bodies when they collide.
 * The bodies should be destroyed by calling body_remove().
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_destructive_collision(scene_t *scene, body_t *body1, body_t *body2);

/**
 * Adds a force creator to a scene that destroys only the first body when they
 * collide.
 *
 * @param scene the scene containing the bodies
 * @param body1 the first body
 * @param body2 the second body
 */
void create_half_destructive_collision(scene_t *scene, double elasticity,
                                       body_t *body1, body_t *body2);
/**
 * Adds a force creator to a scene that applies impulses
 * to resolve collisions between two bodies in the scene.
 * This should be represented as an on-collision callback
 * registered with create_collision().
 *
 * You may remember from project01 that you should avoid applying impulses
 * multiple times while the bodies are still colliding.
 * You should also have a special case that allows either body1 or body2
 * to have mass INFINITY, as this is useful for simulating walls.
 *
 * @param scene the scene containing the bodies
 * @param elasticity the "coefficient of restitution" of the collision;
 * 0 is a perfectly inelastic collision and 1 is a perfectly elastic collision
 * @param body1 the first body
 * @param body2 the second body
 */
void create_physics_collision(scene_t *scene, double elasticity, body_t *body1,
                              body_t *body2);

#endif // #ifndef __FORCES_H__
