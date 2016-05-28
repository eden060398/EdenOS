#ifndef STDDEF_H
#define STDDEF_H
	#include <stddef.h>
#endif
#ifndef STDBOOL_H
#define STDBOOL_H
	#include <stdbool.h>
#endif
#ifndef SYSTEM_H
#define SYSTEM_H
	#include <system.h>
#endif

#define MAX_COMMANDS 100
#define MAX_ARGS 100
#define CMD_NAME_MAX 30
#define MAX_INPUT 1000

#define ESC_SC 0x01
#define PRTSCN_SC 0x37

#define TYPE_UNDEFINED			0
#define TYPE_DIR				1
#define TYPE_FILE				2

#define CALL_TYPE_NORMAL		0
#define CALL_TYPE_HELP			1
#define CALL_TYPE_DESC			2


// -----------------------------------------------------------------------------
// Command-Prompt Module
// ---------------------
// 
// General	:	The module initializes and initiates the command-prompt.
//
// Input	:	A command and parameters: command [param1] [param2] ...
//
// Process	:	Find the matching function for the command in the database and run 
// 				it with the received parameters.
//
// Output	:	The output of the command.
//
// -----------------------------------------------------------------------------
// Programmer	:	Eden Frenkel
// -----------------------------------------------------------------------------

struct command
{
	char name[CMD_NAME_MAX + 1];
	void (*func)(int argc, char **args, int call_type);
};

static struct command commands[MAX_COMMANDS];
static int next;


// COMMANDS DECLARATIONS
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

// -----------------------------------------------------------------------------
// init_command_prompt
// -------------------
// 
// General		:	The function initializes and initiates the command-prompt.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void init_command_prompt(void)
{	
	// Initialize the command-counter.
	next = 0;
	
	// Add commands.
	add_command("help", &help_cmd);
	add_command("clear", &clear_cmd);
	add_command("args", &print_args_cmd);
	add_command("write", &free_text_cmd);
	add_command("mmap", &mmap_cmd);
	add_command("pview", &proc_view_cmd);
	add_command("inf", &inf_loop_cmd);
	add_command("killth", &kill_thread_cmd);
	add_command("edev", &enum_dev_cmd);
	add_command("list", &list_cmd);
	add_command("cd", &change_dir_cmd);
	add_command("read", &read_cmd);
	add_command("del", &delete_cmd);
	add_command("mkdir", &make_dir_cmd);
	
	// Initiate the command-prompt.
	command_prompt();
}

// -----------------------------------------------------------------------------
// add_command
// -----------
// 
// General		:	The function adds a command to the database, so it could
//					used via the command-prompt.
//
// Parameters	:
//		name	-	A pointer to a string that represents the commands name (In)
//		func	-	A pointer to the function that should be called once the
//					command is used (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void add_command(char *name, void (*func)(int argc, char **args, int call_type))
{
	strcpy(commands[next].name, name);
	commands[next].func = func;
	next++;
}

// -----------------------------------------------------------------------------
// exec_command
// ------------
// 
// General		:	The function executes a command and passes the arguments to
//					it.
//
// Parameters	:
//		name	-	A pointer to a string that represents the commands name (In)
//		argc	-	The number of parameters that are passed to the function (In)
//		args	-	A pointer to an array of pointers to strings that are the
//					command's parameters (In)
//		w		-	An int that specipies whether to wait for the command to
//					finish execution; If non-zero - wait, otherwise - proceed (In)
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void exec_command(char *name, int argc, char **args, int w)
{
	int i, j, title_len, cmd_name_len;
	bool found;
	uint32_t *status;
	
	for (i = 0, found = false; i < next && !found; i++)
	{
		if (!strcmp(commands[i].name, name))
		{
			if (w)
			{
				// Add the command's name to the title.
				title_len = strlen(TITLE);
				cmd_name_len = strlen(commands[i].name);
				puts_at(" - ", 0, title_len);
				puts_at(commands[i].name, 0, title_len + 3);
			}
			if (new_thread(name, &status))
			{
				// Within a new thread, call the function and pass the arguments
				// to it.
				(*commands[i].func)(argc, args, CALL_TYPE_NORMAL);
				// When the operation completes, dispose of the thread.
				dispose_thread();
			}
			if (w)
			{
				// Wait for the command to complete.
				wait(status);
				// Remove the command's name from the title.
				for (j = title_len; j < title_len + cmd_name_len + 3; j++)
					putc_at('\x00', 0, j);
			}
			found = true;
		}
	}
	
	if (!found)
		puts("Command not found.\n");
}

// -----------------------------------------------------------------------------
// parse_input
// -----------
// 
// General		:	The function parses the input and divides it to a command
//					name and arguments.
//
// Parameters	:
//		input	-	A pointer to a string that is the input (In)
//		name	-	A pointer to a pointer to a string that will be filled with
//					the name of the command (Out)
//		args	-	A pointer to an array of pointers to strings that will be
//					filled with the command's parameters (Out)
//
// Return Value	:	The amount of parameters that args contains
//
// -----------------------------------------------------------------------------
int parse_input(char *input, char **name, char **args)
{
	int argc;
	
	argc = 0;
	// Ignore spaces at the beginning.
	while (*input == ' ')
		input++;
	// Point 'name' to the beginning of the first word. 
	*name = input++;
	// Proceed to the next space or null.
	while (*input != ' ' && *input)
		input++;
	// If not already null (must be space), replace with null.
	if (*input)
		*input++ = 0;
	while (*input)
	{
		// Ignore spaces.
		while (*input == ' ')
			input++;
		if (*input)
		{
			// Point the next argument pointer to the next word that was found.
			*args++ = input++;
			// Increase the argument pointer.
			argc++;
			// Proceed to the next space or null.
			while (*input != ' ' && *input)
				input++;
			// If not already null (must be space), replace with null.
			if (*input)
				*input++ = 0;
		}
	}
	
	return argc;
}

// -----------------------------------------------------------------------------
// command_prompt
// --------------
// 
// General		:	The function actually initiates and runs the command-prompt.
//
// Parameters	:	None
//
// Return Value	:	None
//
// -----------------------------------------------------------------------------
void command_prompt(void)
{
	char input[MAX_INPUT], *name, *args[MAX_ARGS];
	int argc, w;
	
	while (1)
	{
		puts(working_dir);
		puts(" >>> ");
		gets(input, MAX_INPUT);
		// Set not to wait if the first char of the input is '~'.
		w = input[0] != '~';
		// If set not to wait, ignore the first char of the input ('~').
		argc = w ? parse_input(input, &name, args) :  parse_input(input + 1, &name, args);
		exec_command(name, argc, args, w);
	}
}

// COMMANDS

void help_cmd(int argc, char **args, int call_type)
{
	uint32_t i;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("PRINT HELP INFORMATION\n");
			return;
		case CALL_TYPE_DESC:
			puts("[COMMAND] [COMMAND] [COMMAND] . . .\n");
			puts("\tCOMMAND\tPRINT INFORMATION ON THIS SPECIFIC COMMAND\n");		
			return;
	}
	
	if (!argc)
	{
		for (i = 0; i < next; i++)
		{
			puts(commands[i].name);
			putc('\t');
			(*commands[i].func)(0, 0, CALL_TYPE_HELP);
		}
		return;
	}
	
	while (argc--)
	{
		for (i = 0; i < next; i++)
		{
			if (!strcmp(commands[i].name, *args))
			{
				puts(commands[i].name);
				putc(' ');
				(*commands[i].func)(0, 0, CALL_TYPE_DESC);
				args++;
				break;
			}
		}
	}
}

void clear_cmd(int argc, char **args, int call_type)
{
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("CLEAR THE SCREEN\n");
			return;
		case CALL_TYPE_DESC:
			putc('\n');
			return;
	}
	
	clear_screen();
}

void print_args_cmd(int argc, char **args, int call_type)
{
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("PRINT THE GIVEN ARGUMENTS\n");
			return;
		case CALL_TYPE_DESC:
			puts("[ARG] [ARG] [ARG] . . .\n");
			puts("\tARG\tAN ARGUMENT TO BE PRINTED\n");
			return;
	}
	
	while (argc--)
	{
		puts(*args++);
		putc('\n');
	}
}

void free_text_cmd(int argc, char **args, int call_type)
{
	int c;
	uint32_t n;
	char *path;
	File *f;
	char *buff;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("WRITE TEXT AND SAVE IT TO A FILE\n");
			return;
		case CALL_TYPE_DESC:
			puts("[PATH]\n");
			puts("\tPATH\tTHE PATH OF THE FILE TO SAVE THE TEXT TO\n");
			return;
	}
	
	path = 0;
	while (argc--)
	{
		path = get_full_path(*args);
		f = open(path, strlen(path), 'w');
		free(path);
		if (f)
			break;	
	}
	
	if (path && !f)
	{
		puts("Path not found!\n");
		return;
	}
	
	buff = (char *) palloc();
	n = 0;
	while ((c = getc()) != ESC_SC << 8)
	{
		if (c == PRTSCN_SC << 8)
			while (n > 0)
			{
				putc('\b');
				*(buff + n) = 0;
				n--;
			}
		else if (c == '\b')
		{
			if (n > 0)
			{
				putc(c);
				*(buff + n) = 0;
				n--;
			}
		}
		else if (c < 256)
		{
			if (n < 4096)
			{
				putc(c);
				*(buff + n) = (char) c;
				n++;
			}
		}
	}
	*(buff + n) = 0;
	if (f)
	{
		write_to_file(f, buff, n);
		free((void *) f);
	}
	pfree((void *) buff);
	putc('\n');
}

void mmap_cmd(int argc, char **args, int call_type)
{
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("PRINT THE MEMORY MAPPING\n");
			return;
		case CALL_TYPE_DESC:
			putc('\n');
			return;
	}
	
	print_bios_mmap();
}

void proc_view_cmd(int argc, char **args, int call_type)
{
	int with_pid, with_ppid, with_status, with_threads, with_tid, with_tstatus;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("PRINT INFORMATION ABOUT THE RUNNING PROCESSES\n");
			return;
		case CALL_TYPE_DESC:
			puts("[pid] [ppid] [status] [threads] [tid] [tstatus]\n");
			puts("\tpid\tPRINT THE PROCESS ID OF EACH PROCESS\n");
			puts("\tppid\tPRINT THE PARENT PROCESS ID OF EACH PROCESS\n");
			puts("\tstatus\tPRINT THE STATUS OF EACH PROCESS\n");
			puts("\tthreads\tPRINT THE THREADS OF EACH PROCESS\n");
			puts("\ttid\tPRINT THE THREAD ID OF EACH THREAD\n");
			puts("\ttstatus\tPRINT THE STATUS OF EACH THREAD\n");
			return;
	}
	
	with_pid = with_ppid = with_status = with_threads = with_tid = with_tstatus = 0;
	while (argc--)
	{
		if (!strcmp(*args, "pid"))
			with_pid = 1;
		else if (!strcmp(*args, "ppid"))
			with_ppid = 1;
		else if (!strcmp(*args, "status"))
			with_status = 1;
		else if (!strcmp(*args, "threads"))
			with_threads = 1;
		else if (!strcmp(*args, "tid"))
			with_tid = 1;
		else if (!strcmp(*args, "tstatus"))
			with_tstatus = 1;
		args++;
	}
	print_proc_data(with_pid, with_ppid, with_status, with_threads, with_tid, with_tstatus);
}

void inf_loop_cmd(int argc, char **args, int call_type)
{
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("RUN AN INFINITE LOOP\n");
			return;
		case CALL_TYPE_DESC:
			putc('\n');
			return;
	}
	
	set_idle();
	while (1) ;
}

void kill_thread_cmd(int argc, char **args, int call_type)
{
	uint32_t pid, tid;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("KILL A RUNNING THREAD\n");
			return;
		case CALL_TYPE_DESC:
			puts("pid=PID tid=TID\n");
			puts("\tPID\tTHE PROCESS ID OF THE THREAD'S PROCESS\n");
			puts("\tTID\tTHE THREAD ID OF THE THREAD\n");
			return;
	}
	
	pid = tid = -1;	
	while (argc--)
	{
		if (startswith(*args, "pid="))
		{
			pid = atoui(*args + 4);
		}
		else if (startswith(*args, "tid="))
		{
			tid = atoui(*args + 4);
		}
		args++;
	}
	if (pid != -1 && tid != -1)
		kill_th(pid, tid);
	else
		puts("Invalid parameters.\n");
}

void enum_dev_cmd(int argc, char **args, int call_type)
{
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("LIST THE DEVICES OF THE SYSTEM\n");
			return;
		case CALL_TYPE_DESC:
			putc('\n');
			return;
	}
	
	print_enum_dev();
}

void list_cmd(int argc, char **args, int call_type)
{
	int tree, size;
	char *path;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("LIST DIRECTORY CONTENTS\n");
			return;
		case CALL_TYPE_DESC:
			puts("[tree] [size] [path=PATH]\n");
			puts("\ttree\tPRINT DIRECTORY STRUCTURE AS A TREE\n");
			puts("\tsize\tPRINT THE SIZE OF EACH ITEM\n");
			puts("\tPATH\tTHE PATH OF THE DIRECTORY TO LIST (DEFAULT: WORKING DIRECTORY)\n");
			return;
	}
	
	path = working_dir;
	tree = 0;
	size = 0;
	while (argc--)
	{
		if (startswith(*args, "path="))
			path = *args + 5;
		else if (!strcmp(*args, "tree"))
			tree = 1;
		else if (!strcmp(*args, "size"))
			size = 1;
		args++;
	}
	list(path, strlen(path), tree, size);
}

void change_dir_cmd(int argc, char **args, int call_type)
{	
	char *path;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("CHANGE THE WORKING DIRECTORY\n");
			return;
		case CALL_TYPE_DESC:
			puts("PATH\n");
			puts("\tPATH\tTHE PATH OF THE NEW WORKING DIRECTORY\n");
			return;
	}
	
	if (argc >= 1)
	{
		path = get_full_path(*args);
		if (is_path(path, strlen(path), TYPE_DIR, 0))
		{
			free((void *) working_dir);
			working_dir = path;
			return;
		}
		free(path);
	}
	puts("Path not found!\n");
}

void read_cmd(int argc, char **args, int call_type)
{
	char *path;
	File *f;
	char *buff;
	uint32_t i, len;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("READ A FILE\n");
			return;
		case CALL_TYPE_DESC:
			puts("PATH\n");
			puts("\tPATH\tTHE PATH OF THE FILE TO READ\n");
			return;
	}
	
	f = 0;
	while (argc--)
	{
		path = get_full_path(*args);
		if (is_path(path, strlen(path), TYPE_FILE, 0))
		{
			f = open(path, strlen(path), 'r');
			break;
		}
		free(path);
	}
	if (!f)
	{
		puts("Path not found!\n");
		return;
	}
	clear_screen();
	buff = (char *) palloc();
	while ((len = read_from_file(f, 4096, buff)))
	{
		f->r_seek += len;
		for (i = 0; i < len; i++)
		{
			if (get_current() / 80 >= 21)
			{
				set_color(RED, BLACK);
				puts("\nPRESS ANY KEY TO CONTINUE");
				set_color(LIGHT_GRAY, BLACK);
				getc();
				clear_screen();
			}
			putc(*(buff + i));
		}
	}
	
	free((void *) f);
	pfree((void *) buff);
	putc('\n');
}

void delete_cmd(int argc, char **args, int call_type)
{
	char *path;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("DELETE ITEMS\n");
			return;
		case CALL_TYPE_DESC:
			puts("[PATH] [PATH] [PATH] . . .\n");
			puts("\tPATH\tTHE PATH OF THE TARGET TO DELETE\n");
			return;
	}
	
	while (argc--)
	{
		path = get_full_path(*args);
		if (is_path(path, strlen(path), TYPE_UNDEFINED, 0))
		{
			delete(path, strlen(path));
		}
		else
		{
			puts(*args);
			puts(" not found!\n");
		}
		args++;
		free(path);
	}
	
}

void make_dir_cmd(int argc, char **args, int call_type)
{
	char *path;
	int status;
	
	switch (call_type)
	{
		case CALL_TYPE_HELP:
			puts("CREATE DIRECTORIES\n");
			return;
		case CALL_TYPE_DESC:
			puts("[PATH] [PATH] [PATH] . . .\n");
			puts("\tPATH\tTHE PATH OF THE DIRECTORY TO BE CREATED\n");
			return;
	}
	
	while (argc--)
	{
		path = get_full_path(*args);
		status = create(path, strlen(path), TYPE_DIR);
		if (status)
		{
			puts("Path not found!\n");
		}
		free(path);
		args++;
	}
}