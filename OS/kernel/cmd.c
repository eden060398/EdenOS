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
	void (*func)(int argc, char **args);
};

static struct command commands[MAX_COMMANDS];
static int next;


// COMMANDS DECLARATIONS
void clear_cmd(int argc, char **args);
void print_args_cmd(int argc, char **args);
void free_text_cmd(int argc, char **args);
void mmap_cmd(int argc, char **args);
void just_palloc_cmd(int argc, char **args);
void proc_view_cmd(int argc, char **args);
void inf_loop_cmd(int argc, char **args);
void kill_thread_cmd(int argc, char **args);
void enum_dev_cmd(int argc, char **args);
/*
void print_mdev_port_cmd(int argc, char **args);
void read_main_dev_cmd(int argc, char **args);
void write_main_dev_cmd(int argc, char **args);
void enum_usb_cmd(int argc, char **args);
*/
void list_cmd(int argc, char **args);
void change_dir_cmd(int argc, char **args);
void read_cmd(int argc, char **args);

// FUNCTION DECLARATIONS
void init_command_prompt(void);
void add_command(char *name, void (*func)(int argc, char **args));
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
	add_command("clear", &clear_cmd);
	add_command("print-args", &print_args_cmd);
	add_command("free-text", &free_text_cmd);
	add_command("mmap", &mmap_cmd);
	add_command("just-palloc", &just_palloc_cmd);
	add_command("proc-view", &proc_view_cmd);
	add_command("inf-loop", &inf_loop_cmd);
	add_command("kill-thread", &kill_thread_cmd);
	add_command("edev", &enum_dev_cmd);
	/*
	add_command("main-dev", &print_mdev_port_cmd);
	add_command("read-main-dev", &read_main_dev_cmd);
	add_command("write-main-dev", &write_main_dev_cmd);
	add_command("enum-usb", &enum_usb_cmd);
	*/
	add_command("list", &list_cmd);
	add_command("cd", &change_dir_cmd);
	add_command("read", &read_cmd);
	
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
void add_command(char *name, void (*func)(int argc, char **args))
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
				(*commands[i].func)(argc, args);
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

void clear_cmd(int argc, char **args)
{
	clear_screen();
}

void print_args_cmd(int argc, char **args)
{
	while (argc--)
	{
		puts(*args++);
		putc('\n');
	}
}

void free_text_cmd(int argc, char **args)
{
	int c;
	uint32_t n;
	char *path;
	File *f;
	char *buff;
	
	path = 0;
	while (argc--)
	{
		if (startswith(*args, "path="))
		{
			path = *args + 5;
			f = open(path, strlen(path), 'w');
			if (f)
				break;
		}
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

void mmap_cmd(int argc, char **args)
{
	print_bios_mmap();
}

void just_palloc_cmd(int argc, char **args)
{
	char *buff;
	buff = malloc(100);
	puts(itoa((int) palloc(), buff, 16));
	putc('\n');
	free(buff);
}

void proc_view_cmd(int argc, char **args)
{
	int with_pid, with_ppid, with_status, with_threads, with_tid, with_tstatus;
	
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

void inf_loop_cmd(int argc, char **args)
{
	set_idle();
	while (1) ;
}

void kill_thread_cmd(int argc, char **args)
{
	uint32_t pid, tid;
	
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

void enum_dev_cmd(int argc, char **args)
{
	print_enum_dev();
}

/*
void print_mdev_port_cmd(int argc, char **args)
{
	char *buff;
	
	if (main_dev)
	{
		buff = malloc(10);
		puts("0x");
		puts(uitoa(main_dev->base_port, buff, BASE16));
		putc('\n');
		free(buff);
	}
}

void read_main_dev_cmd(int argc, char **args)
{
	uint32_t block, i;
	void *ptr;
	uint8_t *ptr8;
	char *buff;
	
	if (main_dev && argc == 1)
	{
		block = atoui(*args);
		
		ptr = malloc(512);
		read_bbb(main_dev, block, 1, ptr);
		ptr8 = ptr;
		buff = malloc(5);
		for (i = 0; i < 512; i++)
		{
			if (*ptr8 <= 0xF)
				putc('0');
			puts(uitoa(*ptr8++, buff, BASE16));
		}
		putc('\n');
		free(buff);
		//free(ptr);
	}
}

void write_main_dev_cmd(int argc, char **args)
{
	uint32_t block;
	void *ptr;
	int result;
	char *buff;
	
	if (main_dev && argc == 1)
	{
		block = atoui(*args);
		
		ptr = malloc(512);
		//read_bbb(main_dev, 0, 1, ptr);
		*((uint16_t *) (ptr)) = 0xEDE2;
		
		result = write_bbb(main_dev, block, 1, ptr);
		
		buff = malloc(10);
		puts(uitoa(result, buff, BASE16));
		putc('\n');
		free(buff);
		free(ptr);
	}
}

void enum_usb_cmd(int argc, char **args)
{
	UHCIDevice *temp;
	
	temp = main_dev;
	while (temp)
	{
		putc(temp->tag);
		putc('\n');
		temp = temp->next;
	}
}
*/

void list_cmd(int argc, char **args)
{
	int tree, size;
	char *path;
	
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

void change_dir_cmd(int argc, char **args)
{	
	if (argc >= 1)
	{
		if (**args == '/')
		{
			if (strlen(*args) == 1)
			{
				free((void *) working_dir);
				working_dir = (char *) malloc(2);
				*working_dir = '/';
			}
			
			else if (!is_path(*args, strlen(*args), TYPE_DIR, 0))
			{
				free((void *) working_dir);
				working_dir = (char *) malloc(strlen(*args));
				strcpy(working_dir, *args);
			}
		}
	}
}

void read_cmd(int argc, char **args)
{
	char *path;
	File *f;
	char *buff;
	uint32_t i, len;
	
	path = 0;
	while (argc--)
	{
		if (startswith(*args, "path="))
		{
			path = *args + 5;
			f = open(path, strlen(path), 'r');
			if (f)
				break;
		}
	}
	if (!f)
		return;
	
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