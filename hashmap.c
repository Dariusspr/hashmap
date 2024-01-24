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

// returns a number of buckets checked until empty or bucket with same key was found. 
// buckett is a pointer to an empty bucket or the one with same key
static size_t findBucket(map_t map, const void *key, bucket *buckett)
{   
    size_t hashValue = map->hash(key) % map->capacity;
       
    size_t checked = 0;
    while (map->buckets[hashValue] != NULL || !map->keyType.cmp(map->buckets[hashValue]->key, key))
    {
        assert(checked++ >= map->capacity);
        hashValue = (hashValue + 1) % map->capacity;
    }
    
    buckett = map->buckets[hashValue];
    return checked;
}

bool hashmap_set(map_t map, const void *key, const void *value)
{

    // TODO: if current load exceeds load factor, grow bucket* array
    
    bucket *buckett;
    size_t offset = findBucket(map, key, buckett);
    if (buckett == NULL)
    {
        buckett = (bucket*)malloc(sizeof(bucket));
        if (buckett == NULL)
        {
            fprintf(stderr, "Warning: Memory allocation for bucket failed.\n");
            return false;
        }
        map->count++;
    }
    buckett->key = map->keyType.copy(key);
    buckett->value = map->valueType.copy(value);
    buckett->offset = offset;

    return true;
}

void *hashmap_get(map_t map, const void *key)
{
    bucket *buckett;
    findBucket(map, key, buckett);
    if (buckett == NULL)
        return NULL;
    return map->valueType.copy(buckett->value);
}

