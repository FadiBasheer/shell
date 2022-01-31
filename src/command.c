#include <dc_posix/dc_posix_env.h>
#include <command.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void help_parse(regex_t regex_in, regex_t regex_out, regex_t regex_err, char *token, struct state *state) {
    int matched_err, matched_out, matched_in;
    regmatch_t match;

    matched_err = regexec(&regex_err, token, 1, &match, 0);
    matched_out = regexec(&regex_out, token, 1, &match, 0);
    matched_in = regexec(&regex_in, token, 1, &match, 0);

    if (matched_err == 0) {
        char *str;
        regoff_t length;
        length = match.rm_eo - match.rm_so;
        str = malloc(length + 1);
        strncpy(str, &token[match.rm_so], length);
        str[length] = '\0';
        printf("errorrrrrrrrrrrrrrrrrrrrrrrr: %s\n", str);
        state->command->stderr_file = str;
        free(str);

    } else if (matched_out == 0) {
        char *str;
        regoff_t length;
        length = match.rm_eo - match.rm_so;
        str = malloc(length + 1);
        strncpy(str, &token[match.rm_so], length);
        str[length] = '\0';
        printf("outtttttttttttttt: %s\n", str);
        state->command->stdout_file = str;
        free(str);
    } else if (matched_in == 0) {
        char *str;
        regoff_t length;
        length = match.rm_eo - match.rm_so;
        str = malloc(length + 1);
        strncpy(str, &token[match.rm_so], length);
        str[length] = '\0';
        printf("innnnnnnnnnnnnnnnn: %s\n", str);
        state->command->stdin_file = str;
        free(str);
    } else {
        char *str;
        str = strdup(token);
        printf("comanddddd: %s\n", str);
        state->command->command = strdup(str);
        free(str);
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

    state->command->argv = calloc(3, sizeof(char *));

    state->command->argv[0] = NULL;
    state->command->argc = 1;

    regex_t regex_in, regex_out, regex_err;
    int status_in, status_out, status_err;
    char *string;
    status_err = regcomp(&regex_err, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);
    status_in = regcomp(&regex_in, "[ \\t\\f\\v]<.*", REG_EXTENDED);
    status_out = regcomp(&regex_out, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);

    // I need to add this check for each one of them
    if (status_err != 0) {
        printf("not find error\n");
        size_t size;
        char *msg;
        size = regerror(status_err, &regex_err, NULL, 0);
        msg = malloc(size + 1);
        regerror(status_err, &regex_err, msg, size + 1);
        fprintf(stderr, "%s\n", msg);
        free(msg);
        exit(EXIT_FAILURE);
    }

    const char s[2] = " ";
    char *token;

    string = malloc(strlen(command->line) + 1);  // Space for length plus nul

    strcpy(string, command->line);


    printf("%s\n", string);
    /* get the first token */
    token = strtok(string, s);
    //printf("first token: %s\n", token);
    help_parse(regex_in, regex_out, regex_err, token, state);


    /* walk through other tokens */
    while (token != NULL) {
        token = strtok(NULL, s);
        if (!token) {
            return;
        }
        int matched_err, matched_out, matched_in;
        regmatch_t match;

        matched_err = regexec(&regex_err, token, 1, &match, 0);
        matched_out = regexec(&regex_out, token, 1, &match, 0);
        matched_in = regexec(&regex_in, token, 1, &match, 0);

        printf("%d %d %d\n", matched_err, matched_out, matched_in);

        printf("token_in:%s\n", token);
        help_parse(regex_in, regex_out, regex_err, token, state);
    }
}

void destroy_command(const struct dc_posix_env *env, struct command *command) {
    command->line = NULL;
    command->command = NULL;
    command->argv = NULL;
    command->stdin_file = NULL;
    command->stdout_file = NULL;
    command->stderr_file = NULL;
}
