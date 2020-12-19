#include "seqt.h"

#define NOPS    0x2000
#define M1      (NOPS - 1)
#define M2      ((NOPS << 1) - 1)

typedef enum OpCode {DUR, PIT, INS, DEL} OpCode;

static uint32_t stack[NOPS];
static int p, q, r;

static uint32_t
track_index(unsigned track, unsigned index)
{
    return (track << 16) | (index << 4);
}

uint32_t
op_dur(unsigned track, unsigned index, unsigned delta)
{
    uint32_t op = track_index(track, index) | DUR;
    return op | (delta << 20);
}

uint32_t
op_pit(unsigned track, unsigned index, unsigned voice, unsigned delta)
{
    uint32_t op = track_index(track, index) | PIT;
    return op | (voice << 28) | (delta << 20);
}

uint32_t
op_ins(unsigned track, unsigned index, unsigned nrows)
{
    uint32_t op = track_index(track, index) | INS;
    return op | (nrows << 20);
}

uint32_t
op_del(unsigned track, unsigned index, unsigned nrows)
{
    uint32_t op = track_index(track, index) | DEL;
    return op | (nrows << 20);
}

#define inc(i)      (((i) + 1) & M2)
#define dec(i)      (((i) - 1) & M2)
#define isempty     (p == q)
#define isfull      (p == (q ^ NOPS))

static void
push(uint32_t op)
{
    r = 0;
    stack[q & M1] = op;
    q = inc(q);
}

static uint32_t
pop()
{
    r++;
    q = dec(q);
    return stack[q & M1];
}

static uint32_t
unpop()
{
    r--;
    q = inc(q);
    return stack[q & M1];
}

static void
apply(Matrix matrix, uint32_t op, int reverse)
{
    unsigned int track, index, delta, voice, nrows;
    unsigned int args = op >> 20;
    unsigned char duration, cell;
    track = (op >> 16) & 0xF;
    index = (op >>  4) & 0xFFF;
    switch ((OpCode) (op & 3)) {
        case DUR:
            delta = args;
            duration = matrix[index][0][track >> 1];
            duration = track & 1 ? duration & 0x0F : duration >> 4;
            if (reverse)
                duration = (duration - delta) & 0x0F;
            else
                duration = (duration + delta) & 0x0F;
            matrix[index][0][track >> 1] |= duration << ((~track & 1) << 2);
            break;
        case PIT:
            voice = args >> 8;
            delta = args & 0xFF;
            cell = matrix[index][track][voice];
            if (reverse)
                cell = (cell - delta) & 0xFF;
            else
                cell = (cell + delta) & 0xFF;
            matrix[index][track][voice] = cell;
            break;
        case INS:
            /* TODO */
            (void) nrows;
            break;
        case DEL:
            /* TODO */
            (void) nrows;
    }
}

void
doop(Matrix matrix, uint32_t op, int mark)
{
    apply(matrix, op, 0);
    push(op | (((unsigned) mark) << 2));
}

void
undo(Matrix matrix)
{
    uint32_t op;
    while (!isempty) {
        op = pop();
        apply(matrix, op, 1);
        if (op & 4)
            break;
    }
}

void
redo(Matrix matrix)
{
    uint32_t op;
    while (!isfull) {
        op = unpop();
        if (op & 4) {
            (void) pop();
            break;
        }
        apply(matrix, op, 0);
    }
}
