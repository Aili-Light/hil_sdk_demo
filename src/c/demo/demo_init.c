#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "alg_sdk/alg_sdk.h"
#include "alg_sdk/pull.h"
#ifdef __cplusplus
extern "C" {
#endif

int fatal(const char *msg)
{
    fprintf(stderr, "fatal error : %s", msg);
    exit(1);
}

void int_handler(int sig)
{
    // printf("Caught signal : %d\n", sig);
    alg_sdk_stop();
    alg_sdk_stop_notify();

    /* terminate program */
    exit(sig);
}


void push_callback(void *p)
{
    char *msg = (char *)p;
    printf("Notify Message : %s\n", msg);
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 2) && (strcmp (argv[1], "-s") == 0))
    {
        int rc;
        char *appsrc[] = {"--subscribe"};

        /* Setup notify callback */
        rc = alg_sdk_notify(push_callback);
        if (rc < 0)
        {
            fatal("Setup notify failed\n");
        }
        /* end */

        rc = alg_sdk_init_v2(1, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
        alg_sdk_notify_spin_on();
    }
    else
    {
        int rc;
        
        /* Setup notify callback */
        rc = alg_sdk_notify(push_callback);
        if (rc < 0)
        {
            fatal("Setup notify failed\n");
        }
        /* end */

        rc = alg_sdk_init_v2(argc, argv);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
    }
    return 0;
}
#ifdef __cplusplus
}
#endif
