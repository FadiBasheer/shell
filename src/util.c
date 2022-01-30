#include "util.h"
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dc_posix/dc_stdlib.h>

char *get_prompt(const struct dc_posix_env *env, struct dc_error *err) {
    char *value;
    char *str;

    str = (char *) malloc(2);
    strcpy(str, "$ ");

    value = getenv("PS1");
    if (value == NULL || strlen(value) == 0) {
        return str;
    }
    char *value_dup = strdup(value);
    return value_dup;
}

char *get_path(const struct dc_posix_env *env, struct dc_error *err) {
    if (dc_getenv(env, "PATH") == NULL) {
        return NULL;
    }
    char *value = strdup(dc_getenv(env, "PATH"));
    return value;
}

static size_t count(const char *str, int c) {
    size_t num;
    num = 0;
    for (const char *tmp = str; *tmp; tmp++) {
        if (*tmp == c) {
            num++;
        }
    }
    return num;
}

char **parse_path(const struct dc_posix_env *env, struct dc_error *err, const char *path_str) {
    char *str = strdup(path_str);
    char *state;
    char *token;
    size_t num;
    char **list;
    size_t i = 0;
    state = str;

    num = count(str, ':') + 1;
    list = malloc((num + 1) * sizeof(char *));
    while ((token = strtok_r(state, ":", &state)) != NULL) {
        list[i] = strdup(token);
        i++;
    }
    list[i] = NULL;
    free(str);
    return list;
}


void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state) {
    state->current_line = NULL;
    state->current_line_length = 0;
    state->command = NULL;
    state->fatal_error = false;
    err->message = NULL;
    err->file_name = NULL;
    err->function_name = NULL;
    err->line_number = 0;
    err->type = 0;
    err->err_code = 0;
    err->reporter = NULL;
}

/**
 * Display the state values to the given stream.
 *
 * @param env the posix environment.
 * @param state the state to display.
 * @param stream the stream to display the state on,
 */
void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream) {
//    char *str;
//    str = state_to_string(env, err, state);
//    fprintf(stream, "%s\n", str);
//    free(str);
}

char *state_to_string(const struct dc_posix_env *env, struct dc_error *err, const struct state *state) {
    size_t len;
    char *line;

    if (state->current_line == NULL) {
        len = strlen("current_line = NULL");
    } else {
        len = strlen("current_line = \"\"");
        len += state->current_line_length;
    }

    len += strlen(", fatal_error = ");
    // +1 for 0 or 1 for the fatal error and +1 for the null byte
    line = malloc(len + 1 + 1);

    if (state->current_line == NULL) {
        sprintf(line, "current_line = NULL, fatal_error = %d", state->fatal_error);
    } else {
        sprintf(line, "current_line = \"%s\", fatal_error = %d", state->current_line, state->fatal_error);
    }
    return line;
}
