#include "command.h"
#include <dc_posix/dc_posix_env.h>
#include <execute.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


void redirect(const struct dc_posix_env *env, struct dc_error *err) {

}

void run(const struct dc_posix_env *env, struct dc_error *err) {
}

int handle_run_error(const struct dc_posix_env *env, struct dc_error *err) {
    return 0;
}

/**
 * Create a child process, exec the command with any redirection, set the exit code.
 * If there is an err executing the command print an err message.
 * If the command cannot be found set the command->exit_code to 127.
 *
 * @param env the posix environment.
 * @param err the err object
 * @param command the command to execute
 * @param path the directories to search for the command
 */
void execute(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        redirect(env, err);
        if (dc_error_has_error(err)) {
            exit(126);
        }

        run(env, err);
        status = handle_run_error(env, err);
        exit(status);

    } else if (pid < 0) {
        // Error forking
        perror("lsh");

    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}
