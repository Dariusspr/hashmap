#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>

#define DEFAULT_MAP_LOAD 0.7
#define DEFAULT_MAP_GROWTH 2.0

typedef struct hashmap* map_t;
typedef size_t (*hash_t)(const void *);

typedef struct
{
    void *(*copy)(const void *value); // returns a pointer to a copy of value
    bool (*cmp)(const void *value1, const void *value2); // returns true if equal
} argumentType;

// builtin common types
extern argumentType stringType;
extern argumentType intType; 


/* Creates map
 * Returns NULL if failed to create map_t map
 * initialCapacity
 * loadFactor - map grows when (current bucket count / capacity) > loadFactor
 * growth - new capacity = current capcity * growth
 * hashFunction - a pointer to a function that takes const void * as an argument and returns size_t hashValue
 * keyType - argumentType struct containing neccessary function pointers
 * valueType - argumentType struct containing neccessary function pointers
 * */
extern map_t hashmap_create(size_t initialCapacity, float loadFactor, float growth, hash_t hashFunction, argumentType keyType, argumentType valueType);

// Inserts <key, value> pair to hashmap or changes the value of the existing one
// Returns true if successful
extern bool hashmap_set(map_t map, const void *key, const void *value);

// returns a const pointer to a value at key
// returns NULL if key doesn't exist
extern const void *hashmap_get(map_t map, const void *key);

// frees allocated memory for each element in 'map'
// frees alloacated memory for 'map'
extern void hashmap_free(map_t map);


#endif
