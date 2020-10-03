#include <stdlib.h>
#include <stdio.h>

#include "seqt.h"

int ntracks;
char map[MAPSIZE][RECSIZE];
unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

void
print_blank()
{
    int voice;
    for (voice = 0; voice < MAXVOICE; voice++)
        printf("--");
}

void
print_notes(int track, int index, int head)
{
    int voice;
    unsigned char cell;
    for (voice = 0; voice < MAXVOICE; voice++) {
        cell = matrix[index][track][voice];
        if (cell & 0x80)
            head ? printf("%2u", cell & 0x7F) : printf("||");
        else if (cell == REST)
            printf("--");
        else if (cell == CONT)
            printf("||");
        else
            printf("??");
    }
}

#define inside_view (!vscroll && lines && track >= min_track && track < max_track)

void
print_tracks(int vscroll, int lines, int hscroll, int tracks)
{
    int index[MAXTRACK] = {0};
    int count[MAXTRACK] = {0};
    int active_tracks = MAXTRACK-1;
    int min_track, max_track;
    int track, min_count;
    unsigned char duration;
    min_track = hscroll + 1;            /* inclusive */
    max_track = hscroll + tracks + 1;   /* exclusive */
    if (max_track > MAXTRACK)
        max_track = MAXTRACK;
    while (active_tracks) {
        for (track = 1; track < MAXTRACK; track++) {
            if (inside_view)
                printf(" ");
            if (index[track] == MAXINDEX) {
                if (inside_view)
                    print_blank();
                continue;
            }
            if (!count[track]) {
                duration = matrix[index[track]][0][track >> 1];
                duration = track & 1 ? duration & 0x0F : duration >> 4;
                if (!duration) {
                    index[track] = MAXINDEX;
                    active_tracks--;
                    if (inside_view)
                        print_blank();
                    continue;
                }
                /* head */
                if (inside_view)
                    print_notes(track, index[track], 1);
                count[track] = 0x40 >> (duration - 1);
            } else {
                /* tail */
                if (inside_view)
                    print_notes(track, index[track], 0);
            }
        }
        if (vscroll) {
            vscroll--;
        } else if (lines) {
            printf("\n");
            lines--;
        }
        min_count = 0x40;
        for (track = 1; track < MAXTRACK; track++)
            if (index[track] != MAXINDEX && count[track] < min_count)
                min_count = count[track];
        for (track = 1; track < MAXTRACK; track++) {
            count[track] -= min_count;
            if (!count[track])
                index[track]++;
        }
    }
}

int
main()
{
    if (load_txt(stdin) < 0)
        return 1;
    print_tracks(2, 3, 1, 3);
    if (save_txt(stdout) < 0)
        return 2;
    return 0;
}
