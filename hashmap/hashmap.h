#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>
#include <errno.h>

#define DEFAULT_MAP_GROWAT 0.7
#define DEFAULT_MAP_SHRINKAT 0.1
#define DEFAULT_MAP_GROWTH 2.0

/* Macro to call hashmap_create function with default arguments */
#define hashmap_createDefault(capacity, hashFunction, keyType, valueType) \
    hashmap_create((capacity), DEFAULT_MAP_GROWAT, DEFAULT_MAP_SHRINKAT, DEFAULT_MAP_GROWTH, (hashFunction), (keyType), (valueType))

/* Macros to combine function calls with type casts */
#define hashmap_set(map, key, value) \
    _hashmap_set((map), (const void *)(key), (const void *)(value))

/* To use hashmap_get macro you MUST #define HASHMAP_VALUE_CAST <const data type pointer>
 * Example: for int values - #define HASHMAP_VALUE_CAST const int *
 */
#define hashmap_get(map, key) \
    (HASHMAP_VALUE_CAST)_hashmap_get((map), (const void *)(key))

#define hashmap_delete(map, key) \
    _hashmap_delete((map), (const void *)(key))


typedef struct hashmap* map_t;
typedef size_t (*hash_t)(const void *);

typedef struct 
{
    void *key;
    void *value;
    size_t offset; // If offset is equal to zero, there are no collisions at hash index
                   // Else it indicates the difference between original index and current
} bucket;

typedef struct
{
    void *(*copy)(const void *value);   // Returns a pointer a dynamically allocated memory containing a copy of value
                                        // If value is NULL, returns NULL
    bool (*cmp)(const void *value1, const void *value2); // Returns true if equal and value1 != NULL and value2 != NULL
    void (*free)(void *value); // frees type and it's contents
} argumentType;

/* Built-in argument data types */
extern argumentType stringType;
extern argumentType intType; 


/* Creates map
 * Returns NULL and sets errno if fails to create map_t map
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

/* Inserts <key, value> pair to hashmap or changes the value of the existing one
 * Returns false and sets errno if fails
 */
extern bool _hashmap_set(map_t map, const void *key, const void *value);

/* Returns a const pointer to a value at key
 * Returns NULL if key doesn't exist
 * Return NULL and sets errno if fails
 */
extern const void *_hashmap_get(map_t map, const void *key);

/* Returns an array of pointers to buckets(key,value)
 * Modifications to an array will lead to changes in a map - may cause unexpected errors
 * Some pointers may be equal to NULL, therefore, check before dereferencing
 * Returns NULL and sets errno if fails
 */
extern const bucket **hashmap_getAll(map_t map);

/* Returns map's current capacity
 * Return 0 and sets errno if fails
 */
extern size_t hashmap_getCapacity(map_t map);

/* Frees allocated memory for each element in 'map'
 * Frees alloacated memory for 'map'
 * Returns false and sets errno if fails
 */
extern bool hashmap_free(map_t map);

/* Frees allocated memory for element at key
 * Returns true if successfully deleted
 */
extern bool _hashmap_delete(map_t map, const void *key);

/* Prints info about map:
 * Used count
 * Current capacity
 * Collisions count (count of buckets which are not at key index)
 * Returns false and sets errno if fails
 * */
extern bool hashmap_printInfo(map_t map);

#endif // _HASHMAP_H_
