#include "input.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <dc_util/strings.h>
#include <dc_posix/dc_string.h>

/**
 * Read the command line from the user.
 *
 * @param env the posix environment.
 * @param err the error object
 * @param stream The stream to read from (eg. stdin)
 * @param line_size the maximum characters to read.
 * @return The command line that the user entered.
 */
char *read_command_line(const struct dc_posix_env *env, struct dc_error *err, FILE *stream, size_t *line_size) {
//    char str[1024];
//    char *line;
//    size_t len;
//    memset(str, 0, 1024);
//
//  //  fread(str, *line_size, sizeof (char ), stream);
//
//
//   // stream = fmemopen(str, 1024, "wr");
////    getdelim(&line, &len, '\v', stream);
////    printf("\n%s\n", line);
//
//   // printf("\n%s\n", str);
//    line = NULL;
//    len = 0;
//    getdelim(&line, &len, '\n', stream);
////    getdelim(&line, &len, '\v', stream);
//   printf("--\n%s\n--", line);
//    //fwrite()




    char *line;
    size_t len = 0;
    ssize_t read;
    getline(&line, &len, stream);
    line = strdup(dc_str_trim(env, line));
    *line_size = dc_strlen(env, line);
    printf("--\nlineL: %s\n--", line);
    return line;
}
