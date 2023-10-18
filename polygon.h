#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "color.h"
#include "list.h"
#include "vector.h"

/**
 * A growable array of polygons.
 * A list owns all the polygons in it, so it is responsible for free()ing them.
 * This line does two things:
 * - Declares a "struct polygon" type
 * - Makes "polygon_t" an alias for "struct polygon_list"
 *
 * You will need to implement this struct type in polygon_list.c.
 */
typedef struct polygon polygon_t;

/**
 * Allocates memory for a new list with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param initial_size the number of list elements to allocate space for
 * @return a pointer to the newly allocated list
 */
polygon_t *polygon_init(list_t *star);

/**
 * Allocates memory for a new list with space for the given number of elements.
 * The list is initially empty.
 * Asserts that the required memory was allocated.
 *
 * @param radius radius of the polygon
 * @param num_points number of coordinates the polygon has
 * @param center_x x coordinate center of polygon
 * @param center_y y coordinate center of polygon
 * @return a pointer to the newly allocated list
 */
list_t *polygon_make_star_p(size_t radius, size_t num_points, size_t center_x,
                            size_t center_y);

/**
* Boundary conditions to change velocity when hits bottom of screen
*
* @param polygon the polygon that we are considering
* @param vertices list list of vertices

*/
void polygon_hitting_bottom(polygon_t *polygon, list_t *vertices);

/**
 * Moves the star by translating, rotating, and updating velocity
 * with respect to gravity
 *
 * @param polygon the list of vertices that make up polygon
 * @param time based on last time called
 * @return void
 */
void polygon_star_movement(polygon_t *polygon, double time);

/**
 * Frees all the variables that were malloced
 *
 * @param polygon the list of vertices that make up polygon
 * @return void
 */
void polygon_free(polygon_t *polygon);

/**
 * Returns list of vertices of polygon
 *
 * @param polygon the polygon that we are considering
 * @return list_t list of vertices
 */
list_t *polygon_get_vertices(polygon_t *polygon);

/**
 * Computes the area of a polygon.
 * See https://en.wikipedia.org/wiki/Shoelace_formula#Statement.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the area of the polygon
 */
double polygon_area(list_t *polygon);

/**
 * Computes the center of mass of a polygon.
 * See https://en.wikipedia.org/wiki/Centroid#Of_a_polygon.
 *
 * @param polygon the list of vertices that make up the polygon,
 * listed in a counterclockwise direction. There is an edge between
 * each pair of consecutive vertices, plus one between the first and last.
 * @return the centroid of the polygon
 */
vector_t polygon_centroid(list_t *polygon);

/**
 * Translates all vertices in a polygon by a given vector.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param translation the vector to add to each vertex's position
 */
void polygon_translate(list_t *polygon, vector_t translation);

/**
 * Rotates vertices in a polygon by a given angle about a given point.
 * Note: mutates the original polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @param angle the angle to rotate the polygon, in radians.
 * A positive angle means counterclockwise.
 * @param point the point to rotate around
 */
void polygon_rotate(list_t *polygon, double angle, vector_t point);

/**
 * Gets color of the polygon.
 *
 * @param polygon the list of vertices that make up the polygon
 * @return the color of the polygon
 */
rgb_color_t polygon_get_color(polygon_t *polygon);

#endif // #ifndef __POLYGON_H__
