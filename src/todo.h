#ifndef TODO_TOOL_H
#define TODO_TOOL_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define ANSI_RESET  "\e[0m"
#define ANSI_BOLD   "\e[1m"
#define ANSI_UNDERLINE "\e[4m"
#define ANSI_ITALIC "\e[3m"
#define ANSI_CLEAR  "\e[2J"
#define ANSI_ERASE_LINE "\e[2K"
#define ANSI_HIDE_CURSOR()       printf("\e[?25l")
#define ANSI_SHOW_CURSOR()       printf("\e[?25h")
#define ANSI_GOTOXY(x,y)         printf("\e[%d;%dH", (y), (x))
#define ANSI_MOVE_CURSOR_UP(x)   printf("\e[%zuA", x)
#define ANSI_MOVE_CURSOR_DOWN(x) printf("\e[%dB", x);
#define ANSI_MOVE_CURSOR_RIGHT(x)  printf("\e[%dC", x);
#define ANSI_MOVE_CURSOR_LEFT(x)   printf("\e[%dD", x);
#define ANSI_CLEAR_BELOW_CURSOR()  printf("\e[J")
#define ANSI_CURSOR_BLOCK()        printf("\033[1 q");
#define ANSI_CURSOR_UNDERSCORE()   printf("\033[4 q");
#define ANSI_CURSOR_BAR()          printf("\033[5 q");

// ── Modern color palette (Catppuccin Mocha) ──
// Background colors
#define ANSI_BG_SURFACE  "\e[48;2;49;50;68m"
#define ANSI_BG_OVERLAY  "\e[48;2;69;71;90m"
#define ANSI_BG_BLUE     "\e[48;2;137;180;250m"
#define ANSI_BG_GREEN    "\e[48;2;166;227;161m"
#define ANSI_BG_YELLOW   "\e[48;2;249;226;175m"
#define ANSI_BG_RED      "\e[48;2;243;139;168m"
#define ANSI_BG_PEACH    "\e[48;2;250;179;135m"

// Foreground colors
#define ANSI_FG_WHITE    "\e[38;2;205;214;244m"
#define ANSI_FG_SUBTEXT  "\e[38;2;166;173;200m"
#define ANSI_FG_OVERLAY  "\e[38;2;108;112;134m"
#define ANSI_FG_GRAY     "\e[38;2;147;153;178m"
#define ANSI_FG_BLUE     "\e[38;2;137;180;250m"
#define ANSI_FG_GREEN    "\e[38;2;166;227;161m"
#define ANSI_FG_YELLOW   "\e[38;2;249;226;175m"
#define ANSI_FG_RED      "\e[38;2;243;139;168m"
#define ANSI_FG_PEACH    "\e[38;2;250;179;135m"
#define ANSI_FG_MAUVE    "\e[38;2;203;166;247m"
#define ANSI_FG_TEAL     "\e[38;2;148;226;213m"

// Legacy aliases
#define ANSI_BLACK  "\e[38;2;30;30;46m"
#define ANSI_RED    ANSI_FG_RED
#define ANSI_GREEN  ANSI_FG_GREEN
#define ANSI_YELLOW ANSI_FG_YELLOW
#define ANSI_BLUE   ANSI_FG_BLUE
#define ANSI_PURPLE ANSI_FG_MAUVE
#define ANSI_CYAN   ANSI_FG_TEAL
#define ANSI_LGREY  ANSI_FG_SUBTEXT
#define ANSI_DGREY  ANSI_FG_OVERLAY

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