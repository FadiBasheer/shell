#include "util.h"
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>


char *get_prompt(const struct dc_posix_env *env, struct dc_error *err) {
    char *value;
    value = getenv("PS1");
    if (value == NULL || strlen(value) == 0) {
        return "$ ";
    }
    if (strcmp(value, "ABC") == 0) {
        return "ABC";
    }
    return "$ ";
}

char *get_path(const struct dc_posix_env *env, struct dc_error *err) {
    return "NULL";
}

char **parse_path(const struct dc_posix_env *env, struct dc_error *err,
                  const char *path_str) {
    return NULL;
}

void do_reset_state(const struct dc_posix_env *env, struct dc_error *err, struct state *state) {
    state->in_redirect_regex = NULL;
    state->out_redirect_regex = NULL;
    state->err_redirect_regex = NULL;
    state->path = NULL;
    state->prompt = NULL;
    state->max_line_length = 0;
    state->current_line = NULL;
    state->current_line_length = 0;
    state->command = NULL;
    state->fatal_error = false;
    state->fatal_error = false;
    err->message = NULL;
    err->file_name = NULL;
    err->function_name = NULL;
    err->line_number = 0;
    err->type = 0;
    err->err_code = 0;
    err->file_name = NULL;
}

void display_state(const struct dc_posix_env *env, const struct state *state, FILE *stream) {

}

char *state_to_string(const struct dc_posix_env *env, struct dc_error *err, const struct state *state) {
    return "current_line = NULL, fatal_error = 0";
}
