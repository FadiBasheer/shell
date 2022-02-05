#include "builtins.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dc_util/path.h>
#include <dc_posix/dc_unistd.h>

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
    if (command->argv[1] == NULL) {
        char *path;
        dc_expand_path(env, err, &path, "~");
        if (chdir(path) != 0) {
            perror("lsh1");
        }
    } else {
        if (dc_chdir(env, err, command->argv[1]) != 0) {
            char *path;
            path = strdup(command->argv[1]);

            command->exit_code = 1;
            if (dc_error_is_errno(err, EACCES)) {
                fprintf(errstream, "Search permission is denied for any component of the pathname.");
            }
            if (dc_error_is_errno(err, ELOOP)) {
                fprintf(errstream,
                        "A loop exists in symbolic links encountered during resolution of the path argument");
            }
            if (dc_error_is_errno(err, ENAMETOOLONG)) {
                fprintf(errstream,
                        "The length of a component of a pathname is longer than {NAME_MAX}");
            }
            if (dc_error_is_errno(err, ENOENT)) {
                fprintf(errstream, "%s: does not exist\n", path);
            }
            if (dc_error_is_errno(err, ENOTDIR)) {
                fprintf(errstream, "%s: is not a directory\n", path);
            }
        }
    }
}
