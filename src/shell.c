#include "builtins.h"
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dc_posix/dc_stdlib.h>
#include <shell.h>
#include <dc_util/filesystem.h>
#include <dc_util/path.h>
#include <dc_posix/dc_unistd.h>

/**
 * Run the shell FSM.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the exit code from the shell.
 */
int run_shell(const struct dc_posix_env *env, struct dc_error *error, FILE *in, FILE *out, FILE *err) {
    char *path;
    char *current_working_dir;
    dc_expand_path(env, error, &path, "~");
    printf("%s\n", path);


    current_working_dir = dc_get_working_dir(env, error);
    printf("%s\n", current_working_dir);
    return 0;
}
