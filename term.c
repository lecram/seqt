#include "seqt.h"

#include <string.h>
#include <signal.h>

#define get_terminal_size(TS)   ioctl(0, TIOCGWINSZ, (TS))

#ifndef SIGWINCH
#define SIGWINCH  28
#endif

volatile sig_atomic_t pending_winch;

void
handle_winch(int sig)
{
    (void) sig;
    pending_winch = 1;
}

void
setup_terminal(struct termios *term_prev, struct winsize *term_size)
{
    struct termios term_raw;
    struct sigaction sa;

    /* setup signal handler */
    memset(&sa, 0, sizeof (struct sigaction));
    sa.sa_handler = handle_winch;
    sigaction(SIGWINCH, &sa, NULL);
    /* enter alternate screen buffer */
    printf("\x1B[?47h");
    /* disable echo and canonical mode */
    tcgetattr(0, term_prev);
    term_raw = *term_prev;
    term_raw.c_lflag &= ~(ECHO | ICANON);
    /* blocking read */
    term_raw.c_cc[VMIN] = 1;
    term_raw.c_cc[VTIME] = 0;
    tcsetattr(0, TCSAFLUSH, &term_raw);
    /* get terminal size */
    get_terminal_size(term_size);
}

void
restore_terminal(struct termios *term_prev)
{
    tcsetattr(0, TCSAFLUSH, term_prev);
    printf("\x1B[?47l");
}
