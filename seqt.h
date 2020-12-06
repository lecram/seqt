#ifndef SEQT_H
#define SEQT_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <termios.h>

/* ============================= Matrix ============================= */

#define MAXINDEX    0x1000
#define MAXTRACK    0x0010
#define MAXVOICE    0x0008
extern unsigned char matrix[MAXINDEX][MAXTRACK][MAXVOICE];

#define REST    0
#define CONT    1

/* =============================== Map ============================== */

#define RECSIZE     0x20
#define MAPSIZE     0x20
extern char map[MAPSIZE][RECSIZE];

/* search key in map
 * if key is found, return record index, otherwise:
 *   return the index of the first empty record - MAPSIZE
 *   if map is full, return MAPSIZE */
int map_find(char type, const char *key);

/* search for next record of given type, starting at *rec_index
 * the index found or MAPSIZE is written to the given pointer
 * return the new value of *rec_index */
int map_next(char type, int *rec_index);

/* put a key-value pair on the map
 * if the key is already on the map, update its value
 * return 0 on success, negative value on error:
 *   -1: key-value pair is too large to fit on a record
 *   -2: no record available on the map */
int map_put(char type, const char *key, const char *val);

/* get the value associated with the given key on the map
 * return a pointer to the value found or NULL if not found */
char *map_get(char type, const char *key);


/* ======================== plain-text file ========================= */

#define MAXINPUTLINE    0x400

typedef enum TxtStt {MTDT, TKNM, EVNT} TxtStt;

int load_txt(FILE *fp);
int save_txt(FILE *fp);

/* ============================ terminal =======]==================== */

extern struct winsize term_size;
void setup_terminal(struct termios *term_prev, struct winsize *term_size);
void restore_terminal(struct termios *term_prev);

/* ====================[=====+=====  =====+=====]==================== */

extern int ntracks;

#endif /* SEQT_H */
