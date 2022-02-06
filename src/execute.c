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
            close(input_fds);
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
            fd = open(command->stdout_file, O_RDWR | O_TRUNC | O_CREAT, mode);
        }
        if (dup2(fd, STDOUT_FILENO) < 0) {
            close(fd);
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
            fd = open(command->stderr_file, O_RDWR | O_TRUNC | O_CREAT, mode);
        }
        if (dup2(fd, STDERR_FILENO) < 0) {
            close(fd);
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
                free(cmd);
                break;
            }
        }
    }

}

int handle_run_error(const struct dc_posix_env *env, struct dc_error *err) {
    if (dc_error_is_errno(err, E2BIG)) {
        printf("1111111111111111111");
        return 1;
    } else if (dc_error_is_errno(err, EACCES)) {
        printf("222222222222");
        return 2;
    } else if (dc_error_is_errno(err, EINVAL)) {
        printf("333333333333");
        return 3;
    } else if (dc_error_is_errno(err, ELOOP)) {
        printf("4444444444444");
        return 4;
    } else if (dc_error_is_errno(err, ENAMETOOLONG)) {
        printf("5");
        return 5;
    } else if (dc_error_is_errno(err, ENOENT)) {
        printf("127");
        return 127;
    } else if (dc_error_is_errno(err, ENOTDIR)) {
        printf("5");
        return 6;
    } else if (dc_error_is_errno(err, ENOEXEC)) {
        printf("5");
        return 7;
    } else if (dc_error_is_errno(err, ENOMEM)) {
        printf("5");
        return 8;
    } else if (dc_error_is_errno(err, ETXTBSY)) {
        printf("5");
        return 9;
    } else {
        printf("5");
        return 125;
    }
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
    int pid;
    pid = fork();

    if (pid == 0) {
        int status = 0;
        // Child process
        redirect(env, err, command);
        if (dc_error_has_error(err)) {
            exit(126);
        }

        run(env, err, command, path);
        status = handle_run_error(env, err);
        exit(status);
    } else {
        int status = 0;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            command->exit_code = WEXITSTATUS(status);
        }
    }
}
