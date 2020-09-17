#include <stdlib.h>
#include <stdio.h>

#include "seqt.h"

int ntracks;
char map[MAPSIZE][RECSIZE];
unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

int
main()
{
    if (load_txt(stdin) < 0)
        return -1;
    if (save_txt(stdout) < 0)
        return -2;
    return 0;
}
