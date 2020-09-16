#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAXINDEX    0x1000
#define MAXTRACK    0x0010
#define MAXVOICE    0x0008

#define RECSIZE     0x20
#define MAPSIZE     0x20

#define MAXINPUTLINE    0x400

#define REST    0
#define CONT    1

int ntracks;
char map[MAPSIZE][RECSIZE];
unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

/* search key in map
 * if key is found, return record index, otherwise:
 *   return the index of the first empty record - MAPSIZE
 *   if map is full, return MAPSIZE */
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

/* search for next record of given type, starting at *rec_index
 * the index found or MAPSIZE is written to the given pointer
 * return the new value of *rec_index */
int
map_next(char type, int *rec_index)
{
    for (; *rec_index < MAPSIZE; (*rec_index)++)
        if (map[*rec_index][0] == type)
            break;
    return *rec_index;
}

/* put a key-value pair on the map
 * if the key is already on the map, update its value
 * return 0 on success, negative value on error:
 *   -1: key-value pair is too large to fit on a record
 *   -2: no record available on the map */
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

/* get the value associated with the given key on the map
 * return a pointer to the value found or NULL if not found */
char *
map_get(char type, const char *key)
{
    int i = map_find(type, key);
    if (i < 0 || i == MAPSIZE)
        return NULL;
    return &map[i][strlen(key)+2];
}

typedef enum TxtStt {NTRK, MTDT, TKNM, EVNT} TxtStt;

int
load_txt(FILE *fp)
{
    char line[MAXINPUTLINE];
    char track_key[] = "1";
    TxtStt state = NTRK;
    char *sep, *nl, *cell;
    int track, index, voice;
    unsigned char duration, pitch;
    while (fgets(line, MAXINPUTLINE, fp)) {
        if (line[0] == '\n')
            continue;
        switch (state) {
        case NTRK:
            if (strncmp(line, "ntracks:", 8))
                return -1;
            ntracks = atoi(line + 8);
            track = 1;
            state = MTDT;
            break;
        case MTDT:
            if ((sep = strchr(line, ':'))) {
                nl = strchr(line, '\n');
                *sep = *nl = '\0';
                map_put('#', line, sep + 1);
                break;
            } else {
                state = TKNM;
            }
            /* fall through */
        case TKNM:
            state = EVNT;
track_name:
            nl = strchr(line, '\n');
            *nl = '\0';
            map_put('@', track_key, line);
            *track_key = *track_key == '9' ? 'A' : *track_key + 1;
            index = 0;
            break;
        case EVNT:
            if (isdigit(line[0])) {
                /* noteset */
                duration = (line[0] - '0' + 1) << ((~track & 1) << 2);
                matrix[index][0][track >> 1] |= duration;
                cell = &line[2];
                voice = 0;
                while (*cell) {
                    switch (*cell) {
                    case '.':
                        /* matrix[index][track][voice] = REST; */
                        break;
                    case '=':
                        matrix[index][track][voice] = CONT;
                        break;
                    default:
                        pitch = (cell[0] - '0') * 10 + cell[1] - '0';
                        matrix[index][track][voice] = pitch | 0x80;
                    }
                    cell += 3;
                    voice++;
                }
                index++;
            } else {
                if (!strchr(line, ':')) {
                    track++;
                    goto track_name;
                }
                /* printf("ignoring metaevent: %s", line); */
            }
        }
    }
    ntracks = track;
    return 0;
}

int
save_txt(FILE *fp)
{
    int track, index, voice;
    int last_voice;
    unsigned char cell, duration;
    int rec_index, key_length;
    char track_key[] = "1";
    fprintf(fp, "ntracks:%d\n\n", ntracks);
    for (rec_index = 0; map_next('#', &rec_index) < MAPSIZE; rec_index++) {
        fprintf(fp, "%s:%n", &map[rec_index][1], &key_length);
        fprintf(fp, "%s\n", &map[rec_index][key_length+1]);
    }
    for (track = 1; track <= ntracks; track++) {
        fprintf(fp, "\n%s\n\n", map_get('@', track_key));
        for (index = 0; index < MAXINDEX; index++) {
            duration = matrix[index][0][track >> 1];
            duration = track & 1 ? duration & 0x0F : duration >> 4;
            if (!duration)
                break;
            fprintf(fp, "%c", duration + '0' - 1);
            for (voice = MAXVOICE-1; voice >= 0; voice--)
                if (matrix[index][track][voice])
                    break;
            last_voice = voice;
            for (voice = 0; voice <= last_voice; voice++) {
                cell = matrix[index][track][voice];
                if (cell & 0x80)
                    fprintf(fp, " %2u", cell & 0x7F);
                else if (cell == REST)
                    fprintf(fp, " ..");
                else if (cell == CONT)
                    fprintf(fp, " ==");
                else
                    fprintf(fp, " ??");
            }
            fprintf(fp, "\n");
        }
        *track_key = *track_key == '9' ? 'A' : *track_key + 1;
    }
    return 0;
}

int
main()
{
    if (load_txt(stdin) < 0)
        return -1;
    if (save_txt(stdout) < 0)
        return -2;
    return 0;
}
