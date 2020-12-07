#include "seqt.h"

#include <string.h>
#include <signal.h>

#ifndef SIGWINCH
#define SIGWINCH  28
#endif

/* When a UNIX terminal is resized, it sends a SIGWINCH signal to clients.
 * Normally, seqt's mainloop remains blocked on user input upon SIGWINCH.
 * Setting up any signal handler seems to send EOF to stdin upon SIGWINCH.
 * This allows seqt to redraw the screen immediately. */
void handle_winch(int sig) { (void) sig; }

void
setup_terminal(struct termios *term_prev)
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
}

void
restore_terminal(struct termios *term_prev)
{
    tcsetattr(0, TCSAFLUSH, term_prev);
    printf("\x1B[?47l");
}
