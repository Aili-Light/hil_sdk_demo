#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include "alg_sdk/alg_sdk.h"
#ifdef __cplusplus
extern "C" {
#endif

void int_handler(int sig)
{
    // printf("Caught signal : %d\n", sig);
    alg_sdk_stop();

    /* terminate program */
    exit(sig);
}

int main (int argc, char **argv)
{
    signal(SIGINT, int_handler);

    if ((argc == 2) && (strcmp (argv[1], "-s") == 0))
    {
        int rc;
        char *appsrc[] = {"--subscribe"};
        rc = alg_sdk_init_v2(1, &appsrc[0]);
        if(rc < 0)
        {
            printf("Init SDK Failed\n");
            exit(0);
        }

        alg_sdk_spin_on();
    }
    else
    {
        int rc;
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
