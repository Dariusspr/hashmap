#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

    map->buckets = (bucket **)calloc(capacity, sizeof(bucket *));
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

// returns a number of buckets checked until empty spot/already existing bucket was found. 
// buckett is a pointer to an empty bucket or the one with identical key
static size_t findBucket(map_t map, const void *key, bucket ***buckett)
{   
    size_t hashValue = map->hash(key) % map->capacity; 
    size_t checked = 0;
    
    if (map->buckets[hashValue] != NULL)
    {
        while (!map->keyType.cmp(map->buckets[hashValue]->key, key))
        {
            assert(checked++ >= map->capacity);
            hashValue = (hashValue + 1) % map->capacity;
       }
    }
    *buckett = &(map->buckets[hashValue]);
    return checked;
}

bool hashmap_set(map_t map, const void *key, const void *value)
{
    
    // TODO: if current load exceeds load factor, grow bucket* array   
    
    bucket **buckett = NULL;
    size_t offset = findBucket(map, key, &buckett);
    
    if (*buckett == NULL)
    {
        *buckett = malloc(sizeof(bucket));
        if (*buckett == NULL)
        {
            fprintf(stderr, "Warning: Memory allocation for bucket failed.\n");
            return false;
        }
        map->count++;
        
        (*buckett)->key = map->keyType.copy(key);
        (*buckett)->offset = offset;
    }
    (*buckett)->value = map->valueType.copy(value);
    return true;
}

void *hashmap_get(map_t map, const void *key)
{
    bucket **buckett = NULL;
    findBucket(map, key, &buckett);
    if (*buckett == NULL)
        return NULL;
   
    return map->valueType.copy((*buckett)->value);
}


///////////////////////////// BUILT-IN TYPES ///////////////////////////////////

// STRING TYPE

static void *stringCopy(const void *value)
{
    size_t size = strlen(value) + 1; // +1 for '\0'
    
    char *copy = malloc(size);
    if (copy == NULL)
    {
        fprintf(stderr, "Warning: Memory allocation for string copy failed.\n");
        return NULL;
    }

    memcpy(copy, value, size);
    return copy;
}

static bool stringCmp(const void *value1, const void *value2)
{
    size_t size1 = strlen((const char *)value1);
    size_t size2 = strlen((const char *)value2);
    size_t cmpSize = (size1 < size2) ? size1 : size2;
    return strncmp(value1, value2, cmpSize) == 0;
}

argumentType stringType = {.copy = stringCopy, .cmp = stringCmp};



// INT TYPE

static void *intCopy(const void *value)
{
    int *copy = (int*)malloc(sizeof(int));
    if (copy == NULL)
    {
        fprintf(stderr, "Warning: Memory allocation for int copy failed.\n");
        return NULL;
    }

    *copy = *(const int *)value;

    return copy;
}

static bool intCmp(const void *value1, const void *value2)
{
    return *(const int *) value1 == *(const int *)value2;
}

argumentType intType = {.copy = intCopy, .cmp = intCmp};
