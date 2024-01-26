#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#define DEFAULT_MAP_GROWTHAT 0.7
#define DEFAULT_MAP_SHRINKAT 0.1
#define DEFAULT_MAP_GROWTH 2.0


// TODO: add macros for more convenient use of functions

typedef struct hashmap* map_t;
typedef size_t (*hash_t)(const void *);

typedef struct
{
    void *(*copy)(const void *value); // returns a pointer a dynamically allocated memory containing a copy of value
    bool (*cmp)(const void *value1, const void *value2); // returns true if equal
} argumentType;

// builtin argument data types
extern argumentType stringType;
extern argumentType intType; 


/* Creates map
 * Returns NULL if failed to create map_t map
 * initialCapacity
 * growAt - map grows when (current bucket count / capacity) > growthAt
 * shrinkAt - map shrinks when (current bucket count / capacity) < shrinkAt (shrinkAt must be less than growthAt) and current map capacity > intial capacity
 * growth - value indicating how much the size of map increases or decreases during resizing
 * hashFunction - a pointer to a function that takes const void * as an argument and returns size_t hashValue
 * keyType - argumentType struct containing neccessary function pointers
 * valueType - argumentType struct containing neccessary function pointers
 * */
extern map_t hashmap_create(size_t initialCapacity, float growAt, float shrinkAt, float growth, hash_t hashFunction, argumentType keyType, argumentType valueType);

// Inserts <key, value> pair to hashmap or changes the value of the existing one
// Returns true if successfully set
extern bool hashmap_set(map_t map, const void *key, const void *value);

// returns a const pointer to a value at key
// returns NULL if key doesn't exist
extern const void *hashmap_get(map_t map, const void *key);

// frees allocated memory for each element in 'map'
// frees alloacated memory for 'map'
// returns true if successfully freed
extern bool hashmap_free(map_t map);

// frees allocated memory for element at key
// returns true if successfully deleted
extern bool hashmap_delete(map_t map, const void *key);

/* Prints info about map:
 * Used count
 * Current capacity
 * Collisions count (count of buckets which are not at key index)
 * */
extern void hashmap_printInfo(map_t map);

#endif
