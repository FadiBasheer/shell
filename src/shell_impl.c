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
    int ret_1;
    ret_1 = regcomp(&regex_in, "[ \\t\\f\\v]<.*", 0);
    if (ret_1) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    s->in_redirect_regex = &regex_in;

    regex_t regex_out;
    int ret_2;
    ret_2 = regcomp(&regex_out, "[ \\t\\f\\v][1^2]?>[>]?.*", 0);
    if (ret_2) {
        fprintf(stderr, "Could not compile regex\n");
        exit(1);
    }
    s->out_redirect_regex = &regex_out;

    regex_t regex_err;
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
    s->current_line = NULL;
    s->current_line_length = 0;
    s->max_line_length = (size_t) sysconf(_SC_ARG_MAX);
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
    char *str;
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
    char *working_dir;
    FILE *stream;

    working_dir = dc_get_working_dir(env, err);

    prompt = malloc(1 + strlen(working_dir) + 1 + 2 + strlen(s->prompt) + 1);
    sprintf(prompt, "[%s] %s", dc_get_working_dir(env, err), s->prompt);
    printf("s->prompt2: %s\n", prompt);

    stream = fmemopen(prompt, strlen(prompt) + 1, "r");
    s->stdout = stream;

//    FILE *stream2;
//    stream2 = fmemopen(s->stdin, strlen(s->stdin), "r");
//    s->current_line = stream2;

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
int separate_commands(const struct dc_posix_env *env, struct dc_error *err,
                      void *arg) {
    struct state *s = (struct state *) arg;
    struct command comm;
    s->command = &comm;
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
    return 0;
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
int execute_commands(const struct dc_posix_env *env, struct dc_error *err,
                     void *arg) {
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
    char *str;
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
int handle_error(const struct dc_posix_env *env, struct dc_error *err,
                 void *arg) {
    return DESTROY_STATE;
}
