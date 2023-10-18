#ifndef __SCENE_H__
#define __SCENE_H__

#include "body.h"
#include "list.h"

/**
 * A function called when a collision occurs.
 * @param body1 the first body passed to create_collision()
 * @param body2 the second body passed to create_collision()
 * @param axis a unit vector pointing from body1 towards body2
 *   that defines the direction the two bodies are colliding in
 * @param aux the auxiliary value passed to create_collision()
 */
typedef void (*collision_handler_t)(body_t *body1, body_t *body2, vector_t axis,
                                    void *aux);

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene scene_t;

/**
 * A type that holds the bodies, forcer, aux, and freer
 * to be held by the scene's forces
 */
typedef struct force force_t;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*force_creator_t)(void *aux);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
scene_t *scene_init(void);

/**
 * Releases memory allocated for a given scene
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(scene_t *scene);

/**
 * Frees the force
 *
 * @param force pointer to the force
 */
void force_free(force_t *force);

/**
 * Function to calculate gravity and add to body's force
 *
 * @param aux an auxiliary struct that contains G, body1, body2
 */
void gravity_creator(void *aux);

void normal_creator(void *aux);

/**
 * Function to calculate spring force and add to body's force
 *
 * @param aux an auxiliary struct that contains k, body1, body2
 */
void spring_creator(void *aux);

/**
 * Applies a collision to two bodies and removes one body from the scene
 *
 * @param aux an auxiliary struct that contains k, body1, body2
 */
void half_destructive_collision_creator(body_t *body1, body_t *body2,
                                        vector_t axis, void *aux);

/**
 * Function to calculate impulse and add to body's impulse
 *
 * @param aux an auxiliary struct that contains k, body1, body2
 */

void physics_collision_creator(body_t *body1, body_t *body2, vector_t axis,
                               void *aux);

/**
 * Function to calculate collision force to add to body's force
 *
 * @param aux an auxiliary struct that contains body1, body2
 */

void collision_creator(body_t *body1, body_t *body2, vector_t axis, void *aux);

/**
 * Function to handle collisions based on handler included in aux
 *
 * @param aux an auxiliary structure that contains body1, body2, handler, freer
 */
void generic_collision_creator(void *aux);

/**
 * Function to calculate drag force and add to body's force
 *
 * @param aux an auxiliary struct that contains gamma, body1
 */
void drag_creator(void *aux);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(scene_t *scene);

/**
 * Gets the number of forces in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of forces added with scene_add_force()
 */
size_t scene_forces(scene_t *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
body_t *scene_get_body(scene_t *scene, size_t index);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(scene_t *scene, body_t *body);

/**
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(scene_t *scene, size_t index);

/**
 * Removes the body at a given index directly - not flagged
 */
void scene_remove_body_fake(scene_t *scene, size_t index);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_force_creator(scene_t *scene, force_creator_t forcer, void *aux,
                             free_func_t freer);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 * The force creator is registered with a list of bodies it applies to,
 * so it can be removed when any one of the bodies is removed.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param bodies the list of bodies affected by the force creator.
 *   The force creator will be removed if any of these bodies are removed.
 *   This list does not own the bodies, so its freer should be NULL.
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_bodies_force_creator(scene_t *scene, force_creator_t forcer,
                                    void *aux, list_t *bodies,
                                    free_func_t freer);

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */
void scene_tick(scene_t *scene, double dt);

#endif // #ifndef __SCENE_H__
