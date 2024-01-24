#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "hashmap.h"

typedef struct 
{
    void *key;
    void *value;
    size_t offset;
} bucket;

struct hashmap
{
    bucket **buckets;
    size_t capacity;
    size_t count;

    float loadFactor;
    float growth;

    hash_t hash;
    
    argumentType keyType;
    argumentType valueType;
};


map_t hashmap_create(size_t capacity, float loadFactor, float growth, hash_t hashFunction, argumentType keyType, argumentType valueType)
{
    map_t map = (map_t)malloc(sizeof(struct hashmap));
    if (map == NULL)
    {
        fprintf(stderr, "Warning: Memory allocation for hash map failed.\n");
        return NULL;
    }

    map->buckets = (bucket**)calloc(capacity, sizeof(bucket*));
    if (map->buckets == NULL)
    {
        fprintf(stderr, "Warning: Memory allocation for hash map's buckets failed.\n");
        return NULL;
    }

    // TODO: add validation
    map->capacity = capacity;
    map->loadFactor = loadFactor;
    map->growth = growth;
    map->hash = hashFunction;
    map->keyType = keyType;
    map->valueType = valueType;

    return map;
}


