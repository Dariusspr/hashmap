#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#define DEFAULT_MAP_GROWTHAT 0.7
#define DEFAULT_MAP_SHRINKAT 0.1
#define DEFAULT_MAP_GROWTH 2.0

// Macro to call hashmap_create function with default arguments
#define hashmap_createDefault(capacity, hashFunction, keyType, valueType) \
    hashmap_create((capacity), DEFAULT_MAP_GROWTHAT, DEFAULT_MAP_SHRINKAT, DEFAULT_MAP_GROWTH, (hashFunction), (keyType), (valueType))

// Macros to combine function calls with type casts
#define hashmap_set(map, key, value) \
    _hashmap_set((map), (const void *)(key), (const void *)(value))

// to use hashmap_get macro you must #define HASHMAP_VALUE_CAST <const data type pointer>
// example: for int values - #define HASHMAP_VALUE_CAST const int *
#define hashmap_get(map, key) \
    (HASHMAP_VALUE_CAST)_hashmap_get((map), (const void *)(key))

#define hashmap_delete(map, key) \
    _hashmap_delete((map), (const void *)(key))


typedef struct hashmap* map_t;
typedef size_t (*hash_t)(const void *);

typedef struct
{
    void *(*copy)(const void *value);   // returns a pointer a dynamically allocated memory containing a copy of value
                                        // if value is NULL returns NULL
    bool (*cmp)(const void *value1, const void *value2); // returns true if equal and value1 != NULL and value2 != NULL
} argumentType;

// Built-in argument data types
extern argumentType stringType;
extern argumentType intType; 


/* Creates map
 * Returns NULL if fails to create map_t map
 * - initialCapacity > 1
 * - growAt - map grows when (current bucket count / capacity) > growthAt
 * - shrinkAt - map shrinks when (current bucket count / capacity) < shrinkAt and current map capacity > intial capacity
 * shrinkAt must be less than growAt
 * - growth - value indicating how much the size of map increases or decreases during resizing
 * - hashFunction - a pointer to a function that takes const void * as an argument and returns size_t hashValue
 * - keyType - argumentType struct containing data comparision and copy functions
 * - valueType - argumentType struct containing data comparision and copy functions
 * */
extern map_t hashmap_create(size_t initialCapacity, float growAt, float shrinkAt, 
                            float growth, hash_t hashFunction, argumentType keyType, 
                            argumentType valueType);

// Inserts <key, value> pair to hashmap or changes the value of the existing one
// Returns true if successfully set
extern bool _hashmap_set(map_t map, const void *key, const void *value);

// returns a const pointer to a value at key
// returns NULL if key doesn't exist
extern const void *_hashmap_get(map_t map, const void *key);

// frees allocated memory for each element in 'map'
// frees alloacated memory for 'map'
// returns true if successfully freed
extern bool hashmap_free(map_t map);

// frees allocated memory for element at key
// returns true if successfully deleted
extern bool _hashmap_delete(map_t map, const void *key);

/* Prints info about map:
 * Used count
 * Current capacity
 * Collisions count (count of buckets which are not at key index)
 * */
extern void hashmap_printInfo(map_t map);

#endif
