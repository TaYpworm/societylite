/*
 * This file contains the implementations.  
 * ["ourhdr.h" only contains declarations]
 */

#include "ourhdr.h"
#include <signal.h>

/* Static declations: */
static void err_doit(int errnoflag, const char *fmt, va_list ap);

/*----------------------------------------------------------------------
 | open_max program: copied from p.43 of Stevens
 | 
 | Determine the number of file descriptors 
 ----------------------------------------------------------------------*/
#ifdef OPEN_MAX
static int openmax = OPEN_MAX;
#else
static int openmax = 0;
#endif

#define OPEN_MAX_GUESS 256    /* if OPEN_MAX is indeterminate */
                     /* we're not guaranteed this is adequate */

int open_max(void)
{
    if (openmax == 0) {       /* first time through */
        errno = 0;
        if ( (openmax = sysconf(_SC_OPEN_MAX)) < 0) {
            if (errno == 0)
                openmax = OPEN_MAX_GUESS;       /* it's indeterminate */
            else
                err_sys("sysconf error for _SC_OPEN_MAX");
	}
    }
    return (openmax);
}


/*---------------------------------------------------------------------
 | pr_exit: copied from p. 294 of Stevens 
 | 
 | Print the signal mask for the process
 ---------------------------------------------------------------------*/
void pr_mask(const char *str)
{
    sigset_t   sigset;
    int errno_save;

    errno_save = errno; /* we can be called by signal handlers */
    if (sigprocmask(0, NULL, &sigset) < 0)
        err_sys("sigprocmask error");

    printf("%s", str);

    /* Note: I consulted /usr/include/sys/signal.h for a 
     * full list of all the signals 
     */
    if (sigismember(&sigset, SIGHUP))     printf("SIGHUP");
    if (sigismember(&sigset, SIGINT))     printf("SIGINT");
    if (sigismember(&sigset, SIGQUIT))    printf("SIGQUIT");
    if (sigismember(&sigset, SIGILL))     printf("SIGILL");
    if (sigismember(&sigset, SIGTRAP))    printf("SIGTRAP");
    if (sigismember(&sigset, SIGIOT))     printf("SIGIOT");
    if (sigismember(&sigset, SIGABRT))    printf("SIGABRT");
    //if (sigismember(&sigset, SIGEMT))     printf("SIGEMT");
    if (sigismember(&sigset, SIGFPE))     printf("SIGFPE");
    if (sigismember(&sigset, SIGKILL))    printf("SIGKILL");
    if (sigismember(&sigset, SIGBUS))     printf("SIGBUS");
    if (sigismember(&sigset, SIGSEGV))    printf("SIGSEGV");
    if (sigismember(&sigset, SIGSYS))     printf("SIGSYS");
    if (sigismember(&sigset, SIGPIPE))    printf("SIGPIPE");
    if (sigismember(&sigset, SIGALRM))    printf("SIGALRM");
    if (sigismember(&sigset, SIGTERM))    printf("SIGTERM");
    if (sigismember(&sigset, SIGUSR1))    printf("SIGUSR1");
    if (sigismember(&sigset, SIGUSR2))    printf("SIGUSR2");
    if (sigismember(&sigset, SIGCLD))     printf("SIGCLD");
    if (sigismember(&sigset, SIGCHLD))    printf("SIGCHLD");
    if (sigismember(&sigset, SIGPWR))     printf("SIGPWR");
    if (sigismember(&sigset, SIGWINCH))   printf("SIGWINCH");
    if (sigismember(&sigset, SIGURG))     printf("SIGURG");
    if (sigismember(&sigset, SIGPOLL))    printf("SIGPOLL");
    if (sigismember(&sigset, SIGIO))      printf("SIGIO");
    if (sigismember(&sigset, SIGSTOP))    printf("SIGSTOP");
    if (sigismember(&sigset, SIGTSTP))    printf("SIGTSTP");
    if (sigismember(&sigset, SIGCONT))    printf("SIGCONT");
    if (sigismember(&sigset, SIGTTIN))    printf("SIGTTIN");
    if (sigismember(&sigset, SIGTTOU))    printf("SIGTTOU");
    if (sigismember(&sigset, SIGVTALRM))  printf("SIGVTALRM");
    if (sigismember(&sigset, SIGPROF))    printf("SIGPROF");
    if (sigismember(&sigset, SIGXCPU))    printf("SIGXCPU");
    if (sigismember(&sigset, SIGXFSZ))    printf("SIGXFSZ");
    //if (sigismember(&sigset, SIGWAITING)) printf("SIGWAITING");
    //if (sigismember(&sigset, SIGLWP))     printf("SIGLWP");
    //if (sigismember(&sigset, SIGFREEZE))  printf("SIGFREEZE");
    //if (sigismember(&sigset, SIGTHAW))    printf("SIGTHAW");
    //if (sigismember(&sigset, SIGCANCEL))  printf("SIGCANCEL");
    //if (sigismember(&sigset, SIGLOST))    printf("SIGLOST");
    
    printf("\n");
    errno = errno_save;
}

/*---------------------------------------------------------------------
 | pr_exit: copied from p. 199 of Stevens 
 | 
 | Print a description of the exit status
 ---------------------------------------------------------------------*/
void pr_exit(int status)
{
    if (WIFEXITED(status))
        printf("normal termination, exit status = %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        printf("abnormal termination, signal number = %d%s\n", 
                WTERMSIG(status),
#ifdef WCOREDUMP
                WCOREDUMP(status) ? " (core file generated)" : "");
#else
                "");
#endif
    else if (WIFSTOPPED(status))
        printf("child stopped, signal number = %d\n", WSTOPSIG(status));
}
       


/*---------------------------------------------------------------------
 | err_sys: copied from p. 683 of Stevens
 | 
 | Fatal error related to a system call.
 | Print a message and terminate.
 ---------------------------------------------------------------------*/
void err_sys(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(1, fmt, ap);
    va_end(ap);
    abort();        /* dump core and terminate */
    exit(1);
}


/*---------------------------------------------------------------------
 | err_dump: copied from p. 683 of Stevens
 | 
 | Fatal error related to a system call.
 | Print a message, dump core, and terminate.
 ---------------------------------------------------------------------*/
void err_dump(const char *fmt, ...)
{
    va_list    ap;

    va_start(ap, fmt);
    err_doit(1, fmt, ap);
    va_end(ap);
    abort();       /* dump core and terminate */
    exit(1);       /* shouldn't get here      */
}
 
/*---------------------------------------------------------------------
 | err_quit: copied from p. 683 of Stevens 
 | 
 | Fatal error unrelated to a system call.
 | Print a message and terminate
 ---------------------------------------------------------------------*/
void err_quit (const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    err_doit(0, fmt, ap);
    va_end(ap);
    exit(1);
}


/*---------------------------------------------------------------------
 | err_doit: copied from p. 646 of Stevens. 
 |           Needed for err_sys(), err_quit()
 | 
 | Print a message and return to caller.
 | Caller specifies "errnoflag"
 ---------------------------------------------------------------------*/
static void err_doit(int errnoflag, const char *fmt, va_list ap)
{
    int  errno_save;
    char buf[MAXLINE];
    
    errno_save = errno;    /* value caller might want printed */
    vsprintf(buf, fmt, ap);
    if (errnoflag)
        sprintf(buf+strlen(buf), ": %s", strerror(errno_save));
    strcat(buf, "\n");
    fflush(stdout);        /* in case stdout and stderr are the same */
    fputs(buf, stderr);
    fflush(NULL);          /* flushes all stdio output streams */
    return;
}



