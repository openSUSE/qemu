#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>

#ifdef __x86_64__
#define ARCH_NAME "x86_64"
#endif

int main(int argc, char **argv, char **envp)
{
    char *binfmt;
    char **new_argv;

    /*
     * Check if our file name ends with -binfmt
     */
    binfmt = argv[0] + strlen(argv[0]) - strlen("-binfmt");
    if (strcmp(binfmt, "-binfmt")) {
        fprintf(stderr, "%s: Invalid executable name\n", argv[0]);
        exit(1);
    }
    if (argc < 3) {
        fprintf(stderr, "%s: Please use me through binfmt with P flag\n",
                argv[0]);
        exit(1);
    }

    binfmt[0] = '\0';
    /* Now argv[0] is the real qemu binary name */

#ifdef ARCH_NAME
    {
        char *hostbin;
        char *guestarch;
        int r;

        guestarch = strrchr(argv[0], '-') ;
        if (!guestarch) {
            goto skip;
        }
        guestarch++;
        r = asprintf(&hostbin, "/emul/" ARCH_NAME "-for-%s/%s", guestarch, argv[1]);
        if ((r > 0) && !access(hostbin, X_OK)) {
            /*
             * We found a host binary replacement for the non-host binary. Let's
             * use that instead!
             */
            return execve(hostbin, &argv[2], envp);
        }
    }
skip:
#endif

    new_argv = (char **)malloc((argc + 2) * sizeof(*new_argv));
    if (argc > 3) {
        memcpy(&new_argv[4], &argv[3], (argc - 3) * sizeof(*new_argv));
    }
    new_argv[0] = argv[0];
    new_argv[1] = (char *)"-0";
    new_argv[2] = argv[2];
    new_argv[3] = argv[1];
    new_argv[argc + 1] = NULL;

    return execve(new_argv[0], new_argv, envp);
}
