#ifndef TODO_TOOL_H
#define TODO_TOOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ANSI_RESET "\e[0;39m"
#define ANSI_BOLD "\e[1m"
#define ANSI_UNDERLINE "\e[4m"
#define ANSI_ITALIC "\e[3m"
#define ANSI_CLEAR "\e[2J"
#define ANSI_ERASE_LINE "\e[2K"
#define ANSI_HIDE_CURSOR() printf("\e[?25l")
#define ANSI_SHOW_CURSOR() printf("\e[?25h")
#define ANSI_GOTOXY(x,y) printf("\e[%d;%dH", (y), (x))
#define ANSI_MOVE_CURSOR_UP(x) printf("\e[%zuA", x)
#define ANSI_MOVE_CURSOR_DOWN(x) printf("\e[%dB", x);
#define ANSI_MOVE_CURSOR_RIGHT(x) printf("\e[%dC", x);
#define ANSI_MOVE_CURSOR_LEFT(x) printf("\e[%dD", x);
#define ANSI_CLEAR_BELOW_CURSOR() printf("\e[J")
#define ANSI_CURSOR_BLOCK() printf("\033[1 q");
#define ANSI_CURSOR_UNDERSCORE() printf("\033[4 q");
#define ANSI_CURSOR_BAR() printf("\033[5 q");

#define ANSI_BLACK "\e[30m"
#define ANSI_RED "\e[31m"
#define ANSI_GREEN "\e[32m"
#define ANSI_YELLOW "\e[33m"
#define ANSI_BLUE "\e[34m"
#define ANSI_PURPLE "\e[35m"
#define ANSI_CYAN "\e[36m"
#define ANSI_LGREY "\e[37m"
#define ANSI_DGREY "\e[38m"

void enable_input_buffering();
void disable_input_buffering();

typedef enum {
	TASK_TODO,
	TASK_IN_PROGRESS,
	TASK_TESTING,
	TASK_DONE,
	TASK_STATUS_COUNT
} TaskStatus;

void status_tag(TaskStatus status, char buffer[]);

#define TASK_NAME_BUFFER_SIZE 256
#define TASK_DESC_BUFFER_SIZE 1024
typedef struct {
	TaskStatus status;
	size_t priority;
	char name[TASK_NAME_BUFFER_SIZE];
	char description[TASK_DESC_BUFFER_SIZE];
} Task;

#define MAX_TASKS 256

typedef struct {
	Task items[MAX_TASKS];
	size_t count;
} Tasks;

bool task_append(Tasks* tasks, Task task);
bool task_remove(Tasks* tasks, size_t index);
int task_print(Task task, FILE* fd);

bool tasks_save(Tasks* tasks, char* file);
bool tasks_load(Tasks* tasks, char* file);

#define DEFAULT_FILE "TODO.txt"

static inline void strip_newline(char* s)
{
	size_t len = strlen(s);
	if (len > 0 && s[len-1] == '\n') s[len-1] = '\0';
}

#endif // TODO_TOOL_H