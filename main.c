#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "built-in.h"
#include "launch.h"

// Main function
int main(int argc, char** argv)
{
    // Start the shell loop
    ksh_loop();
    return EXIT_SUCCESS;
}
