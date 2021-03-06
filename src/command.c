#include <dc_posix/dc_posix_env.h>
#include <command.h>
#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <string.h>
#include <dc_util/path.h>

static void status_check(int status, regex_t regex) {
    if (status != 0) {
        size_t size;
        char *msg;
        size = regerror(status, &regex, NULL, 0);
        msg = malloc(size + 1);
        regerror(status, &regex, msg, size + 1);
        fprintf(stderr, msg);
        free(msg);
        exit(EXIT_FAILURE);
    }
}

static void
expand_path(const struct dc_posix_env *env, struct dc_error *err, const char *expected_file, char **expanded_file) {
    if (expected_file == NULL) {
        *expanded_file = NULL;
    } else {
        dc_expand_path(env, err, expanded_file, expected_file);
    }
}

/**
 * Parse the command. Take the command->line and use it to fill in all of the fields.
 *
 * @param env the posix environment.
 * @param err the error object.
 * @param state the current state, to set the fatal_error and access the command line and regex for redirection.
 * @param command the command to parse.
 */
void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *command) {

    int matched, matched2;
    regmatch_t match;
    regex_t regex_in, regex_out, regex_err, regex_err2, regex_out2;
    int status_in, status_out, status_err;
    char *string;

    status_err = regcomp(&regex_err, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);
    status_out = regcomp(&regex_out, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);
    status_in = regcomp(&regex_in, "[ \\t\\f\\v]<.*", REG_EXTENDED);

    status_check(status_err, regex_err);
    status_check(status_out, regex_out);
    status_check(status_in, regex_in);

    const char s[2] = " ";

    string = malloc(strlen(command->line) + 1);  // Space for length plus nul

    strcpy(string, command->line);

    /////////////////////////////////////////////////// err ////////////////////////////////////
    matched = regexec(&regex_err, string, 1, &match, 0);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';

        char *ret;
        ret = strstr(str, ">>");
        if (ret)
            state->command->stderr_overwrite = true;


        status_err = regcomp(&regex_err2, "[^> *]*$", REG_EXTENDED);
        status_check(status_err, regex_err2);
        matched2 = regexec(&regex_err2, str, 1, &match, 0);
        if (matched2 == 0) {
            char *str2;
            char *expanded_stderr_file;
            regoff_t length2 = match.rm_eo - match.rm_so;
            str2 = malloc(length2 + 1);
            strncpy(str2, &str[match.rm_so], length2);
            str2[length2] = '\0';
            expand_path(env, err, str2, &expanded_stderr_file);
            command->stderr_file = strdup(expanded_stderr_file);
            free(str2);
        }

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';

        string = strdup(temp);
        free(temp);
        free(str);
    }
    regfree(&regex_err);

    ///////////////////////////////////////////////// out ////////////////////////////////////////
    matched = regexec(&regex_out, string, 1, &match, 0);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';

        char *ret;
        ret = strstr(str, ">>");
        if (ret)
            state->command->stdout_overwrite = true;

        status_err = regcomp(&regex_out2, "[^> ]*$", REG_EXTENDED);


        status_check(status_err, regex_out2);
        matched2 = regexec(&regex_out2, str, 1, &match, 0);

        if (matched2 == 0) {
            char *str2;
            char *expanded_stdout_file;
            regoff_t length2 = match.rm_eo - match.rm_so;
            str2 = malloc(length2 + 1);
            strncpy(str2, &str[match.rm_so], length2);
            str2[length2] = '\0';
            expand_path(env, err, str2, &expanded_stdout_file);
            command->stdout_file = strdup(expanded_stdout_file);
            free(str2);
        }

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';

        string = strdup(temp);
        free(temp);
        free(str);
    }
    regfree(&regex_out);

    ////////////////////////////////////////////////////////////////// IN ///////////////////////
    int flag = 0;
    matched = regexec(&regex_in, string, 1, &match, 0);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';

        status_err = regcomp(&regex_out2, "[^< *]*$", REG_EXTENDED);
        status_check(status_err, regex_out2);
        matched2 = regexec(&regex_out2, str, 1, &match, 0);

        if (matched2 == 0) {
            char *str2;
            char *expanded_stdin_file;
            regoff_t length2 = match.rm_eo - match.rm_so;
            str2 = malloc(length2 + 1);
            strncpy(str2, &str[match.rm_so], length2);
            str2[length2] = '\0';
            expand_path(env, err, str2, &expanded_stdin_file);
            command->stdin_file = strdup(expanded_stdin_file);
            free(str2);
        }

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';

        string = strdup(temp);
        free(temp);
        free(str);
        flag = 1;
    }
    regfree(&regex_in);
    if (flag == 1) {
        wordexp_t p;
        char **w;

        wordexp(string, &p, 0);
        w = p.we_wordv;
        state->command->argv = calloc(p.we_wordc + 2, sizeof(char *));
        state->command->argc = p.we_wordc;

        for (int i = 0; i < p.we_wordc; i++) {
            state->command->argv[i] = strdup(w[i]);
        }
        state->command->argv[0] = NULL;
        state->command->command = strdup(w[0]);
        wordfree(&p);
    } else {
        state->command->command = strdup(string);
        fprintf(stdout, "%s \"%s\"", "enable to parse", string);
    }
    free(string);
}


void destroy_command(const struct dc_posix_env *env, struct command *command) {
    if (command != NULL) {
        free(command->line);
        command->line = NULL;
        free(command->command);
        command->command = NULL;
        free(command->stdin_file);
        command->stdin_file = NULL;

        free(command->stdout_file);
        command->stdout_file = NULL;

        free(command->stderr_file);
        command->stderr_file = NULL;

        if (command->argv != NULL) {
            for (char *c = *command->argv; c; c = *++command->argv) {
                free(c);
            }
        }
        command->argv = NULL;
        command->argc = 0;
        command->stderr_overwrite = false;
        command->stdout_overwrite = false;
        command->exit_code = 0;
    }
}
