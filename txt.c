#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "seqt.h"

int
load_txt(FILE *fp)
{
    char line[MAXINPUTLINE];
    char track_key[] = "1";
    TxtStt state = MTDT;
    char *sep, *nl, *cell;
    int index, track, voice;
    unsigned char duration, pitch;
    track = 1;
    while (fgets(line, MAXINPUTLINE, fp)) {
        if (line[0] == '\n')
            continue;
        switch (state) {
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
    int index, track, voice;
    int last_voice;
    unsigned char cell, duration;
    int rec_index, key_length;
    char track_key[] = "1";
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
