#include "builtins.h"
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dc_posix/dc_stdlib.h>
#include <unistd.h>
#include <dc_util/filesystem.h>
#include <dc_util/path.h>


/**
 * Change the working directory.
 * ~ is converted to the users home directory.
 * - no arguments is converted to the users home directory.
 * The command->exit_code is set to 0 on success or error->errno_code on failure.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param command the command information
 * @param errstream the stream to print error messages to
 */
void builtin_cd(const struct dc_posix_env *env, struct dc_error *err,
                struct command *command, FILE *errstream) {
    printf("fadi path: %s\n", command->argv[1]);
    printf("path: %s\n", dc_get_working_dir(env, err));
    if (command->argv[1] == NULL) {
        char *path;
        dc_expand_path(env, err, &path, "~");
        if (chdir(path) != 0) {
            perror("lsh1");
        }
    } else {
        if (chdir(command->argv[1]) != 0) {
            perror("lsh2");
        }
    }

    printf("path after: %s\n\n\n", dc_get_working_dir(env, err));
}
