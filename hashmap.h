#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <stdbool.h>
#include <stddef.h>

#define DEFAULT_MAP_LOAD 0.7
#define DEFAULT_MAP_GROWTH 2.0

typedef void *any_t;
typedef struct hashmap *map_t;
typedef size_t (*hash_t)(any_t);

typedef struct
{
    any_t (*copy)(const any_t value); // returns a pointer to a copy of value
    bool (*cmp)(const any_t value1, const any_t value2); // returns true if equal
} argumentType;

// builtin types
extern argumentType stringType;
extern argumentType intType; 

// returns map_t object if it is successfully created
extern map_t hashmap_create(size_t initialCapacity, float loadFactor, float growth, hash_t hashFunction, argumentType keyType, argumentType valueType);
extern bool hashmap_set(map_t map, any_t key, any_t value); // returns false if failed
extern const any_t hashmap_get(map_t map, any_t key); // returns NULL if such key doesn't exist
extern void hashmap_free(map_t map);


#endif
