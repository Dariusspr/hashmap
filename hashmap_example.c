/*
    Simple program to test and show basic hash map functionalities.
*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "hashmap/hashmap.h"

#define HASHMAP_VALUE_CAST const int * // to use hashmap_get macro

#define PERSON_COUNT 100 // number of people generated
#define MAX_SAVED_COUNT 10 // max number of saved peoples records for further tests

// macro to validate function return values
#define CHECK_EXIT(condition) \
    do { \
        if (!(condition)) \
        { \
            fprintf(stderr, "%s:%d: %s\n", __FILE__, __LINE__, strerror(errno)); \
            exit(EXIT_FAILURE); \
        } \
    } while(0)


typedef struct
{
    char *firstName;
    char *lastName;
    unsigned int age;
} person;


// HASHING FUNCTION

size_t hashString(char *s, size_t length)
{
    size_t hash = 0;
    for (int i = 0; i < length; ++i)
    {
        hash = hash * (i + 1) + s[i];
    }
    return hash;
}

size_t simpleHashFunction(const void *key)
{
    assert(key != NULL);
    person p = *(const person *)key;
    
    if (p.firstName == NULL || p.lastName == NULL)
    {
        printf("ERROR\n");
    }

    size_t firstLength = strlen(p.firstName);
    size_t lastLength = strlen(p.lastName);
    
    size_t hashValue = 2 * hashString(p.firstName, firstLength);
    hashValue += 3 * hashString(p.lastName, lastLength);
    hashValue += 5 * p.age;

    return hashValue;
}

// NECESSARY FUNCTIONS FOR CUSTOM ARGUMENT DATA TYPE

bool personCmp(const void *value1, const void *value2)
{
    if (value1 == NULL || value2 == NULL)
        return false;
    
    person p1 = *(const person *)value1;
    person p2 = *(const person *)value2;

    if (p1.age != p2.age )
        return false;

    size_t firstLength1 = strlen(p1.firstName);
    size_t lastLength1 = strlen(p1.lastName);
    size_t firstLength2 = strlen(p2.firstName);
    size_t lastLength2 = strlen(p2.lastName);
    
    if (firstLength1 != firstLength2 || lastLength1 != lastLength2)
        return false;

    return strncmp(p1.firstName, p2.firstName, firstLength1) == 0 &&
            strncmp(p1.lastName, p2.lastName, firstLength2) == 0;
}

void *personCopy(const void *value)
{
    if (value == NULL)
        return NULL;
    
    person p = *(const person *)value;
    person *pCopy = (person *)malloc(sizeof(person));
    CHECK_EXIT(pCopy != NULL);
     
    size_t firstLength = strlen(p.firstName) + 1;
    size_t lastLength = strlen(p.lastName) + 1;
    
    pCopy->firstName = malloc(firstLength);
    CHECK_EXIT(pCopy->firstName != NULL);
    pCopy->lastName = malloc(lastLength);
    CHECK_EXIT(pCopy->lastName != NULL);

    memcpy(pCopy->firstName, p.firstName, firstLength);
    memcpy(pCopy->lastName, p.lastName, lastLength);
    pCopy->age = p.age;

    return pCopy;
}

void personFree(void *value)
{
    if (value == NULL)
        return;
    
    free(((person *)value)->firstName);
    ((person *)value)->firstName = NULL;
    free(((person *)value)->lastName);
    ((person *)value)->lastName = NULL;
    free(value);
    value = NULL;
}

int main()
{
    srand(time(NULL));

    // to be able to use custom map key type
    argumentType personType = {.copy = personCopy, .cmp = personCmp, .free = personFree};

    // key = person's info, value = their id(7 digit integer)
    map_t map = hashmap_createDefault(1, simpleHashFunction, personType, intType);
    
    // data to generate random people
    char *firstNames[] = {"John", "Jane", "Robert", "Alice", "Michael", "Emily", "William", "Olivia", "David", "Sophia"};
    char *lastNames[] = {"Smith", "Johnson", "Williams", "Jones", "Brown", "Davis", "Miller", "Wilson", "Moore", "Taylor"};
    
    int savedCount = 0;
    person savedPerson[MAX_SAVED_COUNT];
    int savedId[MAX_SAVED_COUNT];
    
    person tmpPerson;
    int tmpId;
    int tmpIndex;
        
    // TESTING HASHMAP_SET - generating random people(and ids) and inserting into map
    for (int i = 0; i < PERSON_COUNT; ++i)
    {
        do 
        {
            tmpIndex = rand() % (sizeof(firstNames) / sizeof(char *));
            tmpPerson.firstName = firstNames[tmpIndex];
            tmpIndex = rand() % (sizeof(firstNames) / sizeof(char *));
            tmpPerson.lastName = lastNames[tmpIndex];
            tmpPerson.age = rand() % 80 + 1; // [1, 80]
        } while(hashmap_get(map, &tmpPerson) != NULL); // ensures unique keys to avoid overwriting values for testing purposes 
        tmpId = rand() % 9000000 + 1000000;
        
        CHECK_EXIT(hashmap_set(map, &tmpPerson, &tmpId));

        // saving some people for further tests
        if (savedCount < MAX_SAVED_COUNT && (i % (PERSON_COUNT / MAX_SAVED_COUNT) == 0))
        {
            savedPerson[savedCount] = tmpPerson;
            savedId[savedCount] = tmpId;
            ++savedCount;
        }
    }
    
    // TESTING HASHMAP_GET 
    for (int i = 0; i < savedCount; i++)
    {
        const int *tmp = hashmap_get(map, savedPerson + i); 
        CHECK_EXIT(tmp != NULL);
        assert(*tmp == savedId[i]);
    }
    
    // TESTING HASHMAP_DELETE
    for (int i = 1; i <= savedCount; i++)
    {
        if (MAX_SAVED_COUNT % i == 0)
        {
            CHECK_EXIT(hashmap_delete(map, savedPerson + i - 1)); 
            const int *tmp = hashmap_get(map, savedPerson + i - 1); 
            assert(tmp == NULL);

            // testing hashmap_delete() when item doesn't exist
            assert(hashmap_delete(map, savedPerson + i - 1) == false); 
        }
    }
 
    hashmap_printInfo(map);

    hashmap_free(map);
    return 0;
}
