#include <dc_posix/dc_posix_env.h>
#include <command.h>
#include <stdio.h>
#include <wordexp.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>

void parse_command(const struct dc_posix_env *env, struct dc_error *err,
                   struct state *state, struct command *command) {

    printf("line: %s ----comand: %s\n", command->line, command->command);
    regex_t regex;
    int status;
    regmatch_t match;
    int matched;
    const char *string;
    status = regcomp(&regex, "[ \\t\\f\\v]<.*", REG_EXTENDED);
    if (status != 0) {
        size_t size;
        char *msg;
        size = regerror(status, &regex, NULL, 0);
        msg = malloc(size + 1);
        regerror(status, &regex, msg, size + 1);
        fprintf(stderr, "%s\n", msg);
        free(msg);
        exit(EXIT_FAILURE);
    }
    string = command->line;
    matched = regexec(&regex, string, 1, &match, 0);
    if (matched == 0) {
        char *str;
        regoff_t length;
        length = match.rm_eo - match.rm_so;
        str = malloc(length + 1);
        strncpy(str, &string[match.rm_so], length);
        str[length] = '\0';
        printf("%s\n", str);
        free(str);
    }
}
