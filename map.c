#include <string.h>

#include "seqt.h"

int
map_find(char type, const char *key)
{
    int i;
    for (i = 0; i < MAPSIZE; i++) {
        if (!map[i][0])
            return i - MAPSIZE;
        if (map[i][0] != type)
            continue;
        if (!strcmp(&map[i][1], key))
            break;
    }
    return i;
}

int
map_next(char type, int *rec_index)
{
    for (; *rec_index < MAPSIZE; (*rec_index)++)
        if (map[*rec_index][0] == type)
            break;
    return *rec_index;
}

int
map_put(char type, const char *key, const char *val)
{
    int keylen = strlen(key);
    int vallen = strlen(val);
    int i;
    if (keylen + vallen + 3 > RECSIZE)
        return -1;
    i = map_find(type, key);
    if (i == MAPSIZE)
        return -2;
    if (i < 0) {
        i += MAPSIZE;
        map[i][0] = type;
        strcpy(&map[i][1], key);
    }
    strcpy(&map[i][keylen+2], val);
    return 0;
}

char *
map_get(char type, const char *key)
{
    int i = map_find(type, key);
    if (i < 0 || i == MAPSIZE)
        return NULL;
    return &map[i][strlen(key)+2];
}
