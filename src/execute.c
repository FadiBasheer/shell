#include "command.h"
#include <dc_posix/dc_posix_env.h>
#include <execute.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>


void redirect(const struct dc_posix_env *env, struct dc_error *err, struct command *command) {
    if (command->stdin_file != NULL) {
        int input_fds = open(command->stdout_file, O_RDONLY);

        if (dup2(input_fds, STDIN_FILENO) < 0) {
            printf("Unable to duplicate file descriptor.");
            exit(EXIT_FAILURE);
        }
    }

    if (command->stdout_file != NULL) {
        int fd;
        if (command->stdout_overwrite) {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            char *pathname = "/tmp/file";
            fd = open(command->stdout_file, O_RDWR | O_CREAT | O_TRUNC, mode);
        } else {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            char *pathname = "/tmp/file";
            fd = open(command->stdout_file, O_RDWR | O_APPEND, mode);

        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            printf("Unable to duplicate file descriptor.");
            exit(EXIT_FAILURE);
        }
    }

    if (command->stderr_file != NULL) {
        int fd;
        if (command->stderr_overwrite) {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            char *pathname = "/tmp/file";
            fd = open(command->stdout_file, O_RDWR | O_CREAT | O_TRUNC, mode);
        } else {
            mode_t mode = S_IRWXO | S_IRWXG | S_IRWXU;
            char *pathname = "/tmp/file";
            fd = open(command->stdout_file, O_RDWR | O_APPEND, mode);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            printf("Unable to duplicate file descriptor.");
            exit(EXIT_FAILURE);
        }
    }
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
    int status = 0;

    printf("argv[0]: %s\n", path[0]);
    printf("argv[1]: %s\n", path[1]);
    printf("command->argv[0]: %s\n", command->argv[0]);
    printf("command->argv[1]: %s\n", command->argv[1]);
    printf("command->stdout_file: %s\n", command->stdout_file);
    printf("command->stdin_file: %s\n", command->stdin_file);

    pid = fork();
    if (pid == 0) {
        // Child process
        redirect(env, err, command);
        if (dc_error_has_error(err)) {
            exit(126);
        }
        run(env, err);
        status = handle_run_error(env, err);
        exit(status);
    } else {
        status = 0;
        waitpid(pid, &status, 0);
        printf("waitpid: %d\n", status);
        command->exit_code = status;
    }
}
