
#include "state.h"
#include <dc_posix/dc_posix_env.h>
#include <stdio.h>
#include <string.h>
#include <shell.h>
#include <shell_impl.h>
#include <dc_fsm/fsm.h>
#include <dc_posix/dc_time.h>


static struct dc_fsm_transition transitions[] = {
        {DC_FSM_INIT,       INIT_STATE,        init_state},
        {INIT_STATE,        READ_COMMANDS,     read_commands},
        {INIT_STATE,        ERROR,             handle_error},
        {READ_COMMANDS,     RESET_STATE,       reset_state},
        {READ_COMMANDS,     SEPARATE_COMMANDS, separate_commands},
        {READ_COMMANDS,     ERROR,             handle_error},
        {SEPARATE_COMMANDS, PARSE_COMMANDS,    parse_commands},
        {SEPARATE_COMMANDS, ERROR,             handle_error},
        {PARSE_COMMANDS,    EXECUTE_COMMANDS,  execute_commands},
        {PARSE_COMMANDS,    ERROR,             handle_error},
        {EXECUTE_COMMANDS,  RESET_STATE,       reset_state},
        {EXECUTE_COMMANDS,  EXIT,              do_exit},
        {EXECUTE_COMMANDS,  ERROR,             handle_error},
        {RESET_STATE,       READ_COMMANDS,     read_commands},
        {EXIT,              DESTROY_STATE,     destroy_state},
        {ERROR,             RESET_STATE,       reset_state},
        {ERROR,             DESTROY_STATE,     destroy_state},
        {DESTROY_STATE,     DC_FSM_EXIT, NULL}
};

/**
 * Run the shell FSM.
 *
 * @param env the posix environment.
 * @param err the error object
 * @return the exit code from the shell.
 */
int run_shell(const struct dc_posix_env *env, struct dc_error *error, FILE *in, FILE *out, FILE *err) {
    int ret_val = 0;
    struct dc_fsm_info *fsm_info;

    fsm_info = dc_fsm_info_create(env, error, "shell");
    if (dc_error_has_no_error(error)) {
        struct state s;
        s.stdin = in;
        s.stdout = out;
        s.stderr = err;

        int from_state;
        int to_state;

        ret_val = dc_fsm_run(env, error, fsm_info, &from_state, &to_state, &s, transitions);
        dc_fsm_info_destroy(env, &fsm_info);
    }
    return ret_val;
}




