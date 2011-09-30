#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <stdlib.h>


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
