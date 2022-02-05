#include "command.h"
#include <dc_posix/dc_posix_env.h>
#include <execute.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <dc_posix/dc_unistd.h>


void redirect(const struct dc_posix_env *env, struct dc_error *err, struct command *command) {
    if (command->stdin_file != NULL) {
        int input_fds = open(command->stdin_file, O_RDONLY);

        if (dup2(input_fds, STDIN_FILENO) < 0) {
            printf("Unable to duplicate file descriptor11111.");
            exit(EXIT_FAILURE);
        }
        close(input_fds);
    }

    if (command->stdout_file != NULL) {
        int fd;
        if (command->stdout_overwrite) {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            fd = open(command->stdout_file, O_RDWR | O_CREAT | O_APPEND, mode);
        } else {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            fd = open(command->stdout_file, O_RDWR | O_TRUNC, mode);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            printf("Unable to duplicate file descriptor22222.");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }

    if (command->stderr_file != NULL) {
        int fd;
        if (command->stderr_overwrite) {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            fd = open(command->stderr_file, O_RDWR | O_CREAT | O_APPEND, mode);
        } else {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            fd = open(command->stderr_file, O_RDWR | O_TRUNC, mode);
        }
        if (dup2(fd, STDERR_FILENO) < 0) {
            printf("\nUnable to duplicate file descriptor3333.\n\n");
            exit(EXIT_FAILURE);
        }
        close(fd);
    }
}

void run(const struct dc_posix_env *env, struct dc_error *err, struct command *command, char **path) {
    if (strchr(command->command, '/') != NULL) {
        command->argv[0] = command->command;
        dc_execv(env, err, command->command, command->argv);
    } else {
        if (path[0] == NULL) {
            DC_ERROR_RAISE_ERRNO(err, ENOENT);
        }
        for (char *c = *path; c; c = *++path) {
            char *cmd;
            cmd = malloc(1 + strlen(c) + 1 + strlen(command->command));
            sprintf(cmd, "%s/%s", c, command->command);
            command->argv[0] = cmd;
            dc_execv(env, err, cmd, command->argv);
            if (!dc_error_is_errno(err, ENOENT)) {
                break;
            }
        }
    }

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
    pid_t pid;
    int status = 0;

    pid = fork();

    if (pid == 0) {
        // Child process
        redirect(env, err, command);
        if (dc_error_has_error(err)) {
            exit(126);
        }

        run(env, err, command, path);
        status = handle_run_error(env, err);
        exit(status);
    } else {
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("statusst : %d\n", status);
            command->exit_code = WEXITSTATUS(status);
        }
    }
}
