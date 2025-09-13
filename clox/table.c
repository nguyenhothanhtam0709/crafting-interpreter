#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define TABLE_MAX_LOAD 0.75

static Entry *findEntry(Entry *entries, int capacity, ObjString *key)
{
    uint32_t index = key->hash % capacity;
    Entry *tombstone = NULL;

    for (;;)
    {
        Entry *entry = &entries[index];
        if (NULL == entry->key)
        {
            if (IS_NIL(entry->value))
            {
                // Empty entry
                return tombstone != NULL ? tombstone : entry;
            }
            else
            {
                // Found a tombstone
                if (NULL == tombstone)
                {
                    tombstone = entry;
                }
            }
        }
        else if (entry->key == key /* Work with string interning */)
        {
            // found the key
            return entry;
        }

        index = (index + 1) % capacity;
    }
}

static void adjustCapacity(Table *table, int capacity)
{
    Entry *entries = ALLOCATE(Entry, capacity);
    for (int i = 0; i < capacity; i++)
    {
        entries[i].key = NULL;
        entries[i].value = NIL_VAL;
    }

    table->count = 0;
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        if (NULL == entry->key /* ignore all empty and tombstone entries */)
        {
            continue;
        }

        Entry *dest = findEntry(entries, capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        table->count++;
    }

    FREE_ARRAY(Entry, table->entries, table->capacity);
    table->entries = entries;
    table->capacity = capacity;
}

void initTable(Table *table)
{
    INIT_DYNAMIC_ARRAY_STRUCT_COMMON_FIELD(table)
    table->entries = NULL;
}

void freeTable(Table *table)
{
    FREE_ARRAY(Entry, table->entries, table->capacity);
    initTable(table);
}

/**
 * If it finds an entry with that key, it returns true, otherwise it returns false.
 * If the entry exists, the value output parameter points to the resulting value.
 */
bool tableGet(Table *table, ObjString *key, Value *value)
{
    if (table->count == 0)
    {
        return false;
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (NULL == entry->key)
    {
        return false;
    }

    *value = entry->value;
    return true;
}

bool tableSet(Table *table, ObjString *key, Value value)
{
    if (table->count + 1 > table->capacity * TABLE_MAX_LOAD /* grow table when at least 75% full */)
    {
        int capacity = GROW_CAPACITY(table->capacity);
        adjustCapacity(table, capacity);
    }

    Entry *entry = findEntry(table->entries, table->capacity, key);
    bool isNewKey = NULL == entry->key;
    if (isNewKey && IS_NIL(entry->value) /* Found an actual empty (not a tombstone) entry */)
    {
        table->count++;
    }

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool tableDelete(Table *table, ObjString *key)
{
    if (0 == table->count)
    {
        return false;
    }

    // Find the entry.
    Entry *entry = findEntry(table->entries, table->capacity, key);
    if (NULL == entry->key)
    {
        return false;
    }

    // Place a tombstone in the entry
    entry->key = NULL;
    entry->value = BOOL_VAL(true); // Mark an entry as a tombstone
    return true;
}

void tableAddAll(Table *from, Table *to)
{
    for (int i = 0; i < from->capacity; i++)
    {
        Entry *entry = &from->entries[i];
        if (entry->key != NULL)
        {
            tableSet(to, entry->key, entry->value);
        }
    }
}

/**
 * Find a string obj in a hash table
 */
ObjString *tableFindString(Table *table, const char *chars, int length, uint32_t hash)
{
    if (table->count == 0)
    {
        return NULL;
    }

    uint32_t index = hash % table->capacity;
    for (;;)
    {
        Entry *entry = &table->entries[index];
        if (entry->key == NULL)
        {
            // Stop if we find an empty non-tombstone entry.
            if (IS_NIL(entry->value))
            {
                return NULL;
            }
        }
        else if (entry->key->length == length && entry->key->hash == hash && memcmp(entry->key->chars, chars, length) == 0)
        {
            // Found it
            return entry->key;
        }

        index = (index + 1) % table->capacity;
    }
}

void markTable(Table *table)
{
    for (int i = 0; i < table->capacity; i++)
    {
        Entry *entry = &table->entries[i];
        markObject((Obj *)entry->key);
        markValue(entry->value);
    }
}