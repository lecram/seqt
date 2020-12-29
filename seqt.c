#include "seqt.h"

#include <stdlib.h>
#include <stdio.h>

#define TRACK_WIDTH 19

int ntracks;
char map[MAPSIZE][RECSIZE];
Matrix matrix;

void
print_header(int hscroll, int tracks)
{
    char track_key[2] = " ";
    char *track_name;
    int t;
    int min_track, max_track;
    min_track = hscroll + 1;            /* inclusive */
    max_track = hscroll + tracks + 1;   /* exclusive */
    if (max_track > MAXTRACK)
        max_track = MAXTRACK;
    for (t = min_track; t < max_track; t++) {
        *track_key = t < 10 ? '0' + t : 'A' + t - 10;
        track_name = map_get('@', track_key);
        if (track_name)
            printf("|  %-*s", TRACK_WIDTH - 3, track_name);
        else
            printf("|%-*s", TRACK_WIDTH - 1, "");
    }
    printf("\n");
}

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
            head ? printf("%02u", cell & 0x7F) : printf("||");
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

#define PTRACKS     (term_size.ws_col / TRACK_WIDTH)
#define PLINES      (term_size.ws_row - 2)

int
main(int argc, char *argv[])
{
    FILE *fin;
    char *fname;
    struct termios term_prev;
    struct winsize term_size;
    int hscroll, vscroll;
    int curx, cury;
    int running, redraw;
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
    doop(matrix, op_pit(1, 1, 0, 12), 1);
    undo(matrix);
    redo(matrix);
    setup_terminal(&term_prev);
    hscroll = vscroll = curx = cury = 0;
    get_terminal_size(&term_size);
    print_header(hscroll, PTRACKS);
    print_tracks(vscroll, PLINES, hscroll, PTRACKS);
    running = 1;
    while (running) {
        key = getchar();
        redraw = 0;
        switch (key) {
        case 'q':
            running = 0;
            break;
        case EOF:
            get_terminal_size(&term_size);
            redraw = 1;
            break;
        case 'H':
            if (hscroll > 0) {
                hscroll--;
                redraw = 1;
            }
            break;
        case 'J':
            if (vscroll < MAXINDEX - PLINES - 1) {
                vscroll++;
                redraw = 1;
            }
            break;
        case 'K':
            if (vscroll > 0) {
                vscroll--;
                redraw = 1;
            }
            break;
        case 'L':
            if (hscroll < MAXTRACK - PTRACKS - 1) {
                hscroll++;
                redraw = 1;
            }
            break;
        }
        if (redraw) {
            printf("\x1B[2J\x1B[H");
            print_header(hscroll, PTRACKS);
            print_tracks(vscroll, PLINES, hscroll, PTRACKS);
        }
    }
    restore_terminal(&term_prev);
    return 0;
}
