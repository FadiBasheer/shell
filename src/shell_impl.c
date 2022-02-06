#include <dc_posix/dc_stdlib.h>
#include <unistd.h>
#include <dc_util/filesystem.h>
#include <stdlib.h>
#include <string.h>
#include <command.h>
#include "shell_impl.h"
#include "util.h"
#include "shell_impl.h"
#include "state.h"
#include "input.h"
#include "builtins.h"

/**
 * Set up the initial state:
 *  - in_redirect_regex  "[ \t\f\v]<.*"
 *  - out_redirect_regex "[ \t\f\v][1^2]?>[>]?.*"
 *  - err_redirect_regex "[ \t\f\v]2>[>]?.*"
 *  - path the PATH environ var separated into directories
 *  - prompt the PS1 environ var or "$ " if PS1 not set
 *  - max_line_length the value of _SC_ARG_MAX (see sysconf)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS or INIT_ERROR
 */
int init_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    char *str;
    struct state *s = (struct state *) arg;
    regex_t regex_in;
    regex_t regex_out;
    regex_t regex_err;
    int ret_1;

    ret_1 = regcomp(&regex_in, "[ \\t\\f\\v]<.*", 0);
    if (ret_1) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    s->in_redirect_regex = &regex_in;

    int ret_2;
    ret_2 = regcomp(&regex_out, "[ \\t\\f\\v][1^2]?>[>]?.*", 0);
    if (ret_2) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    s->out_redirect_regex = &regex_out;

    int ret_3;
    ret_3 = regcomp(&regex_err, "[ \\t\\f\\v]<.*", 0);
    if (ret_3) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    s->err_redirect_regex = &regex_err;

    str = get_path(env, err);
    s->path = parse_path(env, err, str);

    s->prompt = get_prompt(env, err);
    s->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
    s->current_line = NULL;
    s->current_line_length = 0;
    s->fatal_error = false;
    s->command = NULL;
    return READ_COMMANDS;
}

/**
 * Free any dynamically allocated memory in the state and sets variables to NULL, 0 or false.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DC_FSM_EXIT
 */
int destroy_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    //regfree(s->in_redirect_regex);
    s->in_redirect_regex = NULL;
    s->out_redirect_regex = NULL;
    s->err_redirect_regex = NULL;

    s->command = NULL;
    s->current_line_length = 0;
    s->current_line = NULL;
    s->max_line_length = 0;
    s->path = NULL;
    s->prompt = NULL;
    s->stderr = stderr;
    s->stdout = stdout;
    s->stdin = stdin;
    s->fatal_error = false;
    return DC_FSM_EXIT;
}

/**
 * Reset the state for the next read (see do_reset_state).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return READ_COMMANDS
 */
int reset_state(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    char *str;
    struct state *s = (struct state *) arg;
    do_reset_state(env, err, s);
    return READ_COMMANDS;
}

/**
 * Prompt the user and read the command line (see read_command_line).
 * Sets the state->current_line and current_line_length.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return SEPARATE_COMMANDS
 */
int read_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    char *prompt;
    char *command;
    size_t line_size;
    char *working_dir;

    working_dir = dc_get_working_dir(env, err);
    if (dc_error_has_error(err)) {
        s->fatal_error = true;
        return ERROR;
    }
    prompt = malloc(1 + strlen(working_dir) + 1 + 2 + strlen(s->prompt) + 1);
    sprintf(prompt, "[%s] %s", working_dir, s->prompt);
    size_t prompt_length = strlen(prompt);
    prompt[prompt_length] = '\0';

    fprintf(s->stdout, "%s", prompt);
    // fputs(prompt, s->stdout);

    command = read_command_line(env, err, s->stdin, &line_size);

    if (dc_error_has_error(err)) {
        s->fatal_error = true;
        return ERROR;
    }
    if (strlen(command) == 0) {
        s->current_line = strdup("");
        s->current_line_length = 0;
        return RESET_STATE;
    }
    s->current_line = strdup(command);
    s->current_line_length = line_size;

    free(prompt);
    free(command);
    free(working_dir);

    return SEPARATE_COMMANDS;
}

/**
 * Separate the commands. In the current implementation there is only one command.
 * Sets the state->command.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return PARSE_COMMANDS or SEPARATE_ERROR
 */
int separate_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    s->command = calloc(1, sizeof(struct command));
    s->command->line = strdup(s->current_line);
    s->command->command = NULL;
    s->command->argc = 0;
    s->command->argv = NULL;
    s->command->stdin_file = NULL;
    s->command->stdout_file = NULL;
    s->command->stderr_file = NULL;
    s->command->exit_code = 0;
    return PARSE_COMMANDS;
}

/**
 * Parse the commands (see parse_command)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXECUTE_COMMANDS or PARSE_ERROR
 */
int parse_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    parse_command(env, err, s, s->command);
    if (dc_error_has_error(err)) {
        return ERROR;
    }
    return EXECUTE_COMMANDS;
}


/**
 * Run the command (see execute).
 * If the command->command is cd run builtin_cd
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return EXIT (if command->command is exit), RESET_STATE or EXECUTE_ERROR
 */
int execute_commands(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    if (strcmp(s->command->command, "cd") == 0) {
        builtin_cd(env, err, s->command, s->stderr);
    } else if (strcmp(s->command->command, "exit") == 0) {
        return EXIT;
    } else {
        execute(env, err, s->command, s->path);
        if (dc_error_has_error(err)) {
            s->fatal_error = true;
        }
    }
    fprintf(s->stdout, "%d\n", s->command->exit_code);
    if (s->fatal_error) {
        return ERROR;
    }
    return RESET_STATE;
}


/**
 * Handle the exit command (see do_reset_state)
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return DESTROY_STATE
 */
int do_exit(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    do_reset_state(env, err, s);
    return DESTROY_STATE;
}

/**
 * Print the error->message to stderr and reset the error (see dc_err_reset).
 *
 * @param env the posix environment.
 * @param err the error object
 * @param arg the current struct state
 * @return RESET_STATE or DESTROY_STATE (if state->fatal_error is true)
 */
int handle_error(const struct dc_posix_env *env, struct dc_error *err, void *arg) {
    struct state *s = (struct state *) arg;
    if (s->current_line == NULL) {
        fprintf(s->stderr, "internal error (%d) %s\n", err->err_code, err->message);
    } else {
        fprintf(s->stderr, "internal error (%d) %s: \"%s\"\n", err->err_code, err->message, s->current_line);
    }
    if (s->fatal_error) {
        return DESTROY_STATE;
    }
    return RESET_STATE;
}
