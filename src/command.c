#include <dc_posix/dc_posix_env.h>
#include <command.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


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

    state->command->argc = 1;


    regex_t regex_in, regex_out, regex_err;
    int status_in, status_out, status_err;
    regmatch_t match;
    int matched_err, matched_out, matched_in;
    const char *string;
    status_err = regcomp(&regex_err, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);
    status_in = regcomp(&regex_in, "[ \\t\\f\\v]<.*", REG_EXTENDED);
    status_out = regcomp(&regex_out, "[ \t\f\v][1^2]?>[>]?.*", REG_EXTENDED);

//    if (status != 0) {
//        size_t size;
//        char *msg;
//        size = regerror(status, &regex, NULL, 0);
//        msg = malloc(size + 1);
//        regerror(status, &regex, msg, size + 1);
//        fprintf(stderr, "%s\n", msg);
//        free(msg);
//        exit(EXIT_FAILURE);
//    }

    const char s[2] = " ";
    char *token;

    printf("command->line: %s\n", command->line);
    string = command->line;

    string = malloc(strlen(command->line) + 1);  // Space for length plus nul

    strcpy(string, command->line);

    /* get the first token */
    token = strtok(string, s);


    printf("line1: %s\n", token);

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
        printf("errorrrrrrrrrrrrrrrrrrrrrrrr: %s\n", token);
        state->command->command = token;
    }

    /* walk through other tokens */
//    while (token != NULL) {
//        printf("Token: %s\n", token);
//
//        token = strtok(NULL, s);
//
//        matched_err = regexec(&regex_err, token, 1, &match, 0);
//        matched_out = regexec(&regex_out, token, 1, &match, 0);
//        matched_in = regexec(&regex_in, token, 1, &match, 0);
//        if (matched_err == 0) {
//            char *str;
//            regoff_t length;
//            length = match.rm_eo - match.rm_so;
//            str = malloc(length + 1);
//            strncpy(str, &token[match.rm_so], length);
//            str[length] = '\0';
//            printf("errorrrrrrrrrrrrrrrrrrrrrrrr: %s\n", str);
//            state->command->stderr_file = str;
//            free(str);
//
//        } else if (matched_out == 0) {
//            char *str;
//            regoff_t length;
//            length = match.rm_eo - match.rm_so;
//            str = malloc(length + 1);
//            strncpy(str, &token[match.rm_so], length);
//            str[length] = '\0';
//            printf("outtttttttttttttt: %s\n", str);
//            state->command->stdout_file = str;
//            free(str);
//        } else if (matched_in == 0) {
//            char *str;
//            regoff_t length;
//            length = match.rm_eo - match.rm_so;
//            str = malloc(length + 1);
//            strncpy(str, &token[match.rm_so], length);
//            str[length] = '\0';
//            printf("innnnnnnnnnnnnnnnn: %s\n", str);
//            state->command->stdin_file = str;
//            free(str);
//        } else {
//            printf("errorrrrrrrrrrrrrrrrrrrrrrrr: %s\n", token);
//            state->command->command = token;
//        }
//
//        printf("token_in: %s\n", token);
//    }
    printf("token_end: %s\n", token);

    printf("command->line: %s\n", command->line);

}

void destroy_command(const struct dc_posix_env *env, struct command *command) {

}