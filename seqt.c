#include "seqt.h"

#include <stdlib.h>
#include <stdio.h>

int ntracks;
char map[MAPSIZE][RECSIZE];
unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

void
print_blank()
{
    int voice;
    printf("   ");
    for (voice = 0; voice < MAXVOICE; voice++)
        printf("--");
}

void
print_notes(unsigned char duration, int track, int index, int head)
{
    int voice;
    unsigned char cell;
    printf(" %c ", head ? duration + '0' - 1 : ' ');
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
                    print_notes(duration, track, index[track], 1);
                count[track] = 0x40 >> (duration - 1);
            } else {
                /* tail */
                if (inside_view)
                    print_notes(duration, track, index[track], 0);
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
main(int argc, char *argv[])
{
    FILE *fin;
    char *fname;
    struct termios term_prev;
    struct winsize term_size;
    int running;
    char key;
    if (argc < 2) {
        fprintf(stderr, "usage:\n  %s file\n", argv[0]);
        return 1;
    }
    fname = argv[1];
    fin = fopen(fname, "r");
    if (fin == NULL || load_txt(fin) < 0) {
        fprintf(stderr, "could not read file '%s'\n", fname);
        return 1;
    }
    fclose(fin);
    setup_terminal(&term_prev);
    print_tracks(0, 10, 0, 3);
    running = 1;
    while (running) {
        get_terminal_size(&term_size);
        printf("%hux%hu      \r", term_size.ws_col, term_size.ws_row);
        key = getchar();
        switch (key) {
        case 'q':
            running = 0;
            break;
        }
    }
    restore_terminal(&term_prev);
    return 0;
}
