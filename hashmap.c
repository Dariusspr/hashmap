#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "hashmap.h"

#define CHECK_EXIT(condition) \
    do { \
        if (!(condition)) \
        { \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, strerror(errno)); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)

#define CHECK_RETURN(condition, error, value) \
    do { \
        if (!(condition)) \
        { \
            errno = (error); \
            return (value); \
        } \
    } while(0)


typedef struct 
{
    void *key;
    void *value;
    size_t offset;
} bucket;

struct hashmap
{
    bucket **buckets;
    size_t initialCapacity;
    size_t capacity;
    size_t count;

    float growAt;
    float shrinkAt;
    float growth;

    hash_t hash;
    
    argumentType keyType;
    argumentType valueType;
};


map_t hashmap_create(size_t capacity, float growAt, float shrinkAt, float growth, hash_t hashFunction, argumentType keyType, argumentType valueType)
{
   CHECK_RETURN(capacity != 0 && 
                growAt > 0.0 && growAt <= 1.0 && 
                shrinkAt >= 0.0 && shrinkAt < growAt && 
                growth > 0.0 && 
                hashFunction != NULL,
                EINVAL,
                NULL); 
    
    map_t map = (map_t)malloc(sizeof(struct hashmap));
    CHECK_EXIT(map != NULL);
    
    map->buckets = (bucket **)calloc(capacity, sizeof(bucket *));
    CHECK_EXIT(map->buckets != NULL);

    map->initialCapacity = capacity;
    map->capacity = capacity;
    map->count = 0;
    map->growAt = growAt;
    map->shrinkAt = shrinkAt;
    map->growth = growth;
    map->hash = hashFunction;
    map->keyType = keyType;
    map->valueType = valueType;
    return map;
}

// returns true if current load >= growAt or overflows in the next insert
static bool isMapOverloaded(map_t map)
{
    float currentLoad = (float)map->count / map->capacity;
    float futureLoad = (float)(map->count + 1.0) / map->capacity;
    return currentLoad >= map->growAt || futureLoad > 1.0;
}

// returns true if current load <= shrinkAt and initial capacity < current capacity
static bool isMapUnderloaded(map_t map)
{
    float currentLoad = (float)map->count / map->capacity;
    return currentLoad <= map->shrinkAt && map->initialCapacity < map->capacity;
}

// Rehashes elements to the new map (doesnt transfer tombstones from previous map)
// returns the count of occupied buckets in the new map
static size_t rehashMap(map_t map, bucket ***newBuckets, size_t newCapacity)
{
    size_t count = 0;
    for (size_t i = 0; i < map->capacity; i++)
    {
        if (map->buckets[i] == NULL)
            continue; // skip empty buckets

        size_t hashValue = map->hash(map->buckets[i]->key) % newCapacity;
        size_t checked = 0;
        while ((*newBuckets)[hashValue] != NULL && !map->keyType.cmp((*newBuckets)[hashValue]->key, map->buckets[i]->key))
        {
            assert(checked++ < map->capacity);
            hashValue = (hashValue + 1) % newCapacity;
        }
        
        map->buckets[i]->offset = checked;
        (*newBuckets)[hashValue] = map->buckets[i];
        count++;
    }
    return count;
}

static void resizeMap(map_t map, size_t newCapacity)
{
    bucket** newBuckets = (bucket **)calloc(newCapacity, sizeof(bucket *));
    CHECK_EXIT(newBuckets != NULL);

    size_t newCount = rehashMap(map, &newBuckets, newCapacity);
    free(map->buckets);
    map->buckets = newBuckets;
    map->capacity = newCapacity;
    map->count = newCount;
    
}

// returns a count of buckets that were checked until expected bucket was found
// buckett is a pointer to an empty bucket or the one with identical key
static size_t findBucket(map_t map, const void *key, bucket ***buckett) // * - to modify outside of function's scope, another * to point to bucket* obj.
{  
    size_t checked = 0;
    
    size_t hashValue = map->hash(key) % map->capacity; 
    if (map->buckets[hashValue] != NULL)
    {
        while (map->buckets[hashValue] != NULL && !map->keyType.cmp(map->buckets[hashValue]->key, key))
        {
            if (checked++ >= map->capacity)
            {
                *buckett = NULL;
                return 0;
            }

            hashValue = (hashValue + 1) % map->capacity;
        }
    }
    *buckett = &(map->buckets[hashValue]);
    
    return checked;
}

bool _hashmap_set(map_t map, const void *key, const void *value)
{
    CHECK_RETURN(map != NULL && key != NULL && value != NULL, EINVAL, false);
    
    if (isMapOverloaded(map))
    {
        resizeMap(map, map->capacity * map->growth);
    }

    bucket **buckett = NULL;
    size_t offset = findBucket(map, key, &buckett);
    
    if (*buckett == NULL)
    {
        *buckett = malloc(sizeof(bucket));
        CHECK_EXIT(*malloc != NULL);
        
        (*buckett)->key = map->keyType.copy(key);        
        (*buckett)->offset = offset;
        
        map->count++;
    }
    else
    {
        free((*buckett)->value); // free previous value
    }
    (*buckett)->value = map->valueType.copy(value);       
    
    return true;
}

const void *_hashmap_get(map_t map, const void *key)
{
    CHECK_RETURN(map != NULL && key != NULL, EINVAL, NULL);
    
    bucket **buckett = NULL;
    findBucket(map, key, &buckett);
    if (buckett == NULL)   
        return NULL;

    return (*buckett)->value;
}

bool _hashmap_delete(map_t map, const void *key)
{
    CHECK_RETURN(map != NULL && key != NULL, EINVAL, false);

    if (isMapUnderloaded(map))
    {
        size_t newCapacity = (map->capacity / map->growth < map->initialCapacity) ? map->initialCapacity : map->capacity / map->growth;
        resizeMap(map, newCapacity);
    }

    bucket **buckett = NULL;
    findBucket(map, key, &buckett);
    if (*buckett == NULL)
    {
        return false;
    }

    free((*buckett)->key);
    free((*buckett)->value);
    free(*buckett);
    *buckett = NULL;
    
    map->count--;
    
    return true;
}

bool hashmap_free(map_t map)
{
    CHECK_RETURN(map != NULL, EINVAL, false);

    for (size_t i = 0; i < map->capacity; i++)
    {
        if (map->buckets[i] != NULL)
        {
            free(map->buckets[i]->key);
            free(map->buckets[i]->value);
            free(map->buckets[i]);
        }
    }

    free(map->buckets);
    free(map); 

    return true;
}

static size_t getCollisionCount(map_t map)
{
    size_t collisionCount = 0;
    for (size_t i = 0; i < map->capacity; i++)
    {
        if (map->buckets[i] == NULL)
            continue;

        if (map->buckets[i]->offset != 0)
            collisionCount++;
    }
    return collisionCount;
}

void hashmap_printInfo(map_t map)
{
    if (map == NULL)
    {
        errno = EINVAL;
        return;
    }

    printf("[INFO] used: %zu\n", map->count);
    printf("[INFO] capacity: %zu\n", map->capacity);
    printf("[INFO] collisions: %zu\n",  getCollisionCount(map));
}

///////////////////////////// BUILT-IN TYPES ///////////////////////////////////


// STRING TYPE

static void *stringCopy(const void *value)
{
    if (value == NULL)
        return NULL;
    
    size_t size = strlen(value) + 1; // +1 for '\0'
    char *copy = malloc(size);
    CHECK_EXIT(copy != NULL);
    memcpy(copy, value, size);
    return copy;
}

static bool stringCmp(const void *value1, const void *value2)
{
     if (value1 == NULL || value2 == NULL) 
        return false;
    
    size_t size1 = strlen((const char *)value1);
    size_t size2 = strlen((const char *)value2);
    size_t cmpSize = (size1 < size2) ? size1 : size2;
    return strncmp(value1, value2, cmpSize) == 0;
}

argumentType stringType = {.copy = stringCopy, .cmp = stringCmp};



// INT TYPE

static void *intCopy(const void *value)
{
    if (value == NULL)
        return NULL;

    int *copy = (int*)malloc(sizeof(int));
    CHECK_EXIT(copy != NULL);

    *copy = *(const int *)value;

    return copy;
}

static bool intCmp(const void *value1, const void *value2)
{
    if (value1 == NULL || value2 == NULL) 
        return false;
    
    return *(const int *) value1 == *(const int *)value2;
}

argumentType intType = {.copy = intCopy, .cmp = intCmp};
