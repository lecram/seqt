#include "seqt.h"

void
setup_terminal(struct termios *term_prev, struct winsize *term_size)
{
    struct termios term_raw;

    printf("\x1B[?47h");
    tcgetattr(0, term_prev);
    term_raw = *term_prev;
    term_raw.c_lflag &= ~(ECHO | ICANON);
    /* blocking read */
    term_raw.c_cc[VMIN] = 1;
    term_raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &term_raw);
    /* get terminal size */
    ioctl(0, TIOCGWINSZ, term_size);
}

void
restore_terminal(struct termios *term_prev)
{
    tcsetattr(0, TCSAFLUSH, term_prev);
    printf("\x1B[?47l");
}
