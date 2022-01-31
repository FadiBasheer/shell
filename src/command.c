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
    regex_t regex_in, regex_out, regex_err, regex_err2;
    int status_in, status_out, status_err;
    char *string;


    state->command->argv = calloc(3, sizeof(char *));

    state->command->argv[0] = NULL;
    state->command->argc = 1;


    status_err = regcomp(&regex_err, "[ \\t\\f\\v]2>[>]?.*", REG_EXTENDED);
    status_out = regcomp(&regex_out, "[ \\t\\f\\v][1^2]?>[>]?.*", REG_EXTENDED);
    status_in = regcomp(&regex_in, "[ \\t\\f\\v]<.*", REG_EXTENDED);

    status_check(status_err, regex_err);
    status_check(status_out, regex_out);
    status_check(status_in, regex_in);

    const char s[2] = " ";
    char *token;

    string = malloc(strlen(command->line) + 1);  // Space for length plus nul

    strcpy(string, command->line);

    printf("%s\n", string);

    matched = regexec(&regex_err, string, 1, &match, 0);
    printf("matched:%d\n", matched);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;
        printf("%zu\n", rest);

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
        printf("%s\n", str);

        status_err = regcomp(&regex_err2, "[^>]*$", REG_EXTENDED);
        status_check(status_err, regex_err2);
        matched2 = regexec(&regex_err2, str, 1, &match, 0);
        if (matched2 == 0) {
            char *str2;
            regoff_t length2 = match.rm_eo - match.rm_so;

            str2 = malloc(length2 + 1);
            strncpy(str2, &str[match.rm_so], length2);
            str2[length2] = '\0';
            printf("\nstr2: %s\n\n", str2);
            free(str2);
        }

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';
        printf("string rest: %s\n", temp);

        string = strdup(temp);
        printf("original string: %s\n", string);
        free(temp);
        free(str);
    }
    regfree(&regex_err);

    matched = regexec(&regex_out, string, 1, &match, 0);
    printf("matched:%d\n", matched);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;
        printf("%zu\n", rest);

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
        printf("%s\n", str);

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';
        printf("string rest: %s\n", temp);

        string = strdup(temp);
        printf("original string: %s\n", string);
        free(temp);
        free(str);
    }
    regfree(&regex_out);

    matched = regexec(&regex_in, string, 1, &match, 0);
    printf("matched:%d\n", matched);
    if (matched == 0) {
        char *str;
        regoff_t length = match.rm_eo - match.rm_so;
        size_t rest = strlen(string) - length;
        printf("%zu\n", rest);

        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
        printf("%s\n", str);

        char *temp;
        temp = malloc(rest);
        strncpy(temp, &string[0], rest);
        temp[rest] = '\0';
        printf("string rest: %s\n", temp);

        string = strdup(temp);
        printf("original string: %s\n", string);
        free(temp);
        free(str);
    }
    regfree(&regex_in);

}

void destroy_command(const struct dc_posix_env *env, struct command *command) {
    command->line = NULL;
    command->command = NULL;
    command->argv = NULL;
    command->stdin_file = NULL;
    command->stdout_file = NULL;
    command->stderr_file = NULL;
}
