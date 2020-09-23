#include <stdlib.h>
#include <stdio.h>

#include "seqt.h"

int ntracks;
char map[MAPSIZE][RECSIZE];
unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

void
print_tracks()
{
    int index[MAXTRACK] = {0};
    int count[MAXTRACK] = {0};
    int active_tracks = MAXTRACK-1;
    int i, min_count;
    unsigned char duration;
    while (active_tracks) {
        for (i = 1; i < MAXTRACK; i++) {
            if (index[i] == MAXINDEX) {
                /* TODO: output blank */
                printf("~");
                continue;
            }
            if (!count[i]) {
                duration = matrix[index[i]][0][i >> 1];
                duration = i & 1 ? duration & 0x0F : duration >> 4;
                if (!duration) {
                    index[i] = MAXINDEX;
                    active_tracks--;
                    /* TODO: output blank */
                    printf("~");
                    continue;
                }
                /* head */
                /* TODO: output head */
                printf("*");
                count[i] = 0x40 >> (duration - 1);
            } else {
                /* tail */
                /* TODO: output tail */
                printf("|");
            }
        }
        printf("\n");
        min_count = 0x40;
        for (i = 1; i < MAXTRACK; i++)
            if (index[i] != MAXINDEX && count[i] < min_count)
                min_count = count[i];
        for (i = 1; i < MAXTRACK; i++) {
            count[i] -= min_count;
            if (!count[i])
                index[i]++;
        }
    }
}

int
main()
{
    if (load_txt(stdin) < 0)
        return 1;
    print_tracks();
    if (save_txt(stdout) < 0)
        return 2;
    return 0;
}
