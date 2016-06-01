#ifndef CMD_H
#define CMD_H

#include <system.h>

// DEFINITIONS

#define MAX_COMMANDS 100
#define MAX_ARGS 100
#define CMD_NAME_MAX 30
#define MAX_INPUT 1000

#define ESC_SC 0x01
#define PRTSCN_SC 0x37

#define TYPE_UNDEFINED   0
#define TYPE_DIR    1
#define TYPE_FILE    2

#define CALL_TYPE_NORMAL  0
#define CALL_TYPE_HELP   1
#define CALL_TYPE_DESC   2

// STRUCTURES

struct command {
    char name[CMD_NAME_MAX + 1];
    void (*func)(int argc, char **args, int call_type);
};

// COMMAND DECLARATIONS

void help_cmd(int argc, char **args, int call_type);
void clear_cmd(int argc, char **args, int call_type);
void print_args_cmd(int argc, char **args, int call_type);
void free_text_cmd(int argc, char **args, int call_type);
void mmap_cmd(int argc, char **args, int call_type);
void proc_view_cmd(int argc, char **args, int call_type);
void inf_loop_cmd(int argc, char **args, int call_type);
void kill_thread_cmd(int argc, char **args, int call_type);
void enum_dev_cmd(int argc, char **args, int call_type);
void list_cmd(int argc, char **args, int call_type);
void change_dir_cmd(int argc, char **args, int call_type);
void read_cmd(int argc, char **args, int call_type);
void delete_cmd(int argc, char **args, int call_type);
void make_dir_cmd(int argc, char **args, int call_type);

// FUNCTION DECLARATIONS

void init_command_prompt(void);
void add_command(char *name, void (*func)(int argc, char **args, int call_type));
void exec_command(char *name, int argc, char **args, int w);
int parse_input(char *input, char **name, char **args);
void command_prompt(void);

#endif /* CMD_H */

