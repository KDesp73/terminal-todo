#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <signal.h>
#include <unistd.h>
#include <sys/select.h>
#include "todo.h"

static volatile sig_atomic_t sigint_flag = 0;

static void on_sigint(int sig)
{
	(void)sig;
	sigint_flag = 1;
}

typedef struct {
	size_t active_tab;
	size_t selected_index;
	bool new_task;
	bool running;
} UIState;

enum { KEY_LEFT = 1000, KEY_RIGHT = 1001, KEY_UP = 1002, KEY_DOWN = 1003 };

static int read_edit_key(void)
{
	int c = getchar();
	if (c == -1) return -1;
	if (c != '\e') return c;

	struct timeval tv = {0, 50000};
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(STDIN_FILENO, &fds);

	if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
		c = getchar();
		if (c == -1) return -1;
		if (c == '[' || c == 'O') {
			c = getchar();
			if (c == -1) return -1;
			if (c == 'A') return KEY_UP;
			if (c == 'B') return KEY_DOWN;
			if (c == 'C') return KEY_RIGHT;
			if (c == 'D') return KEY_LEFT;
			return 0;
		}
		return 0;
	}
	return '\e';
}

static bool read_line_inline(char* buffer, size_t buffer_size)
{
	size_t len = strlen(buffer);
	size_t pos = len;

	printf("\e[?25h");
	ANSI_CURSOR_BAR();
	printf("\e[s%s\e[K", buffer);
	fflush(stdout);

	while (true) {
		int key = read_edit_key();
		if (key == -1) break;

		if (key == 10 || key == 13) {
			buffer[len] = '\0';
			printf("\n");
			break;
		}

		if (key == '\e') {
			buffer[0] = '\0';
			printf("\e[?25l");
			return false;
		}

		if (key == 0) continue;

		if (key == 127 || key == 8) {
			if (pos > 0) {
				memmove(&buffer[pos - 1], &buffer[pos], len - pos + 1);
				pos--;
				len--;
				buffer[len] = '\0';
			}
		} else if (key == KEY_RIGHT) {
			if (pos < len) pos++;
		} else if (key == KEY_LEFT) {
			if (pos > 0) pos--;
		} else if (key == KEY_UP || key == KEY_DOWN) {
			/* ignore */
		} else if (key >= 32 && key <= 126) {
			if (len + 1 < buffer_size) {
				memmove(&buffer[pos + 1], &buffer[pos], len - pos + 1);
				buffer[pos] = (char)key;
				pos++;
				len++;
				buffer[len] = '\0';
			}
		}

		printf("\e[u\e[K%s", buffer);
		if (len > pos) printf("\e[%zuD", len - pos);
		fflush(stdout);
	}

	printf("\e[?25l");
	fflush(stdout);
	return true;
}

void tasks_table(Tasks* tasks, UIState* state)
{
	// ── Tab Bar ──
	printf("%s  ", ANSI_BG_SURFACE);
	for (size_t i = 0; i < TASK_STATUS_COUNT; ++i) {
		char tag[5];
		status_tag(i, tag);
		if (state->active_tab == i)
			printf("%s%s%s %s %s%s  ", ANSI_BG_BLUE, ANSI_FG_WHITE, ANSI_BOLD, tag, ANSI_RESET, ANSI_BG_SURFACE);
		else
			printf("%s %s %s  ", ANSI_FG_OVERLAY, tag, ANSI_BG_SURFACE);
		if (i < TASK_STATUS_COUNT - 1)
			printf("%s│%s", ANSI_FG_OVERLAY, ANSI_BG_SURFACE);
	}
	printf("%s\n", ANSI_RESET);

	// ── Task Count ──
	size_t tab_count = 0;
	for (size_t i = 0; i < tasks->count; ++i) {
		if (tasks->items[i].status == state->active_tab)
			tab_count++;
	}
	printf("%s  %zu tasks%s\n\n", ANSI_FG_OVERLAY, tab_count, ANSI_RESET);

	// ── Task List ──
	for (size_t i = 0; i < tasks->count; ++i) {
		Task task = tasks->items[i];
		if (task.status != state->active_tab) continue;

		bool selected = (state->selected_index == i);

		if (selected) {
			printf("%s  %s▶%s ", ANSI_BG_OVERLAY, ANSI_FG_BLUE, ANSI_BG_OVERLAY);
			printf("%s%s%s%s\n", ANSI_FG_WHITE, ANSI_BOLD, task.name, ANSI_RESET);
			if (strlen(task.description) > 0)
				printf("%s   %s%s%s%s\n", ANSI_BG_OVERLAY, ANSI_FG_GRAY, ANSI_ITALIC, task.description, ANSI_RESET);
		} else {
			printf("  %s● %s%s\n", ANSI_FG_OVERLAY, ANSI_FG_SUBTEXT, task.name);
		}
	}

	// ── Footer ──
	printf("\n%s  ", ANSI_FG_OVERLAY);
	for (int i = 0; i < 48; ++i) printf("─");
	printf("%s\n", ANSI_RESET);
	printf("  ");
	printf("%s[q]%s quit  %s[a]%s add  %s[d]%s del  %s[e]%s edit  %s[j/k]%s nav  %s[h/l]%s switch  %s[H/L]%s move%s\n",
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_FG_TEAL, ANSI_FG_OVERLAY,
		ANSI_RESET);
}

static size_t first_visible(Tasks* tasks, TaskStatus status)
{
	for (size_t i = 0; i < tasks->count; ++i)
		if (tasks->items[i].status == status)
			return i;
	return 0;
}

#define SHIFT_UP   165
#define SHIFT_DOWN 166
#define SHIFT_LEFT 167
#define SHIFT_RIGHT 168

static int read_key(void)
{
	int c = getchar();
	if (c == -1) return -1;
	if (c != '\e') return c;

	c = getchar();
	if (c == -1) return -1;
	if (c != '[') return '\e';

	c = getchar();
	if (c == -1) return -1;
	if (c >= 'A' && c <= 'D') return c;
	if (c == 'Z') return 'Z';

	int p1 = 0;
	while (c >= '0' && c <= '9') {
		p1 = p1 * 10 + (c - '0');
		c = getchar();
		if (c == -1) return -1;
	}

	if (c == ';') {
		int p2 = 0;
		c = getchar();
		if (c == -1) return -1;
		while (c >= '0' && c <= '9') {
			p2 = p2 * 10 + (c - '0');
			c = getchar();
			if (c == -1) return -1;
		}
		if (p1 == 1 && p2 == 2) {
			if (c == 'A') return SHIFT_UP;
			if (c == 'B') return SHIFT_DOWN;
			if (c == 'C') return SHIFT_RIGHT;
			if (c == 'D') return SHIFT_LEFT;
		}
		return c;
	}

	return c;
}

int main(int argc, char** argv)
{
	Tasks tasks = {0};

	const char* const files[] = {"todo.txt", "TODO.txt", "TODO", "todo"};
	const char* data_file = files[0];
	bool file_found = false;

	for (size_t i = 0; i < sizeof(files)/sizeof(files[0]); ++i) {
		FILE* f = fopen(files[i], "r");
		if (f) {
			fclose(f);
			data_file = files[i];
			file_found = true;
			break;
		}
	}

	if (file_found && !tasks_load(&tasks, (char*)data_file)) {
		fprintf(stderr, "[ERRO] Failed to load %s. Please fix the file and try again.\n", data_file);
		return 1;
	}

	UIState state = {.running = true};
	state.selected_index = first_visible(&tasks, state.active_tab);

	disable_input_buffering();
	setvbuf(stdin, NULL, _IONBF, 0);
	signal(SIGINT, on_sigint);

	while (state.running) {		
		if (sigint_flag) {
			sigint_flag = 0;
			state.running = false;
			break;
		}

		printf("\e[2J\e[H");
		tasks_table(&tasks, &state);

		int key = read_key();
		if (key == -1) continue;
		switch (key) {
			case 'q': 
			case 3: // Ctrl+C
				state.running = false;
				break;
			case 'l':
			case 67: // RIGHT arrow
			case '\t':
				state.active_tab = (state.active_tab+1) % TASK_STATUS_COUNT;
				state.selected_index = first_visible(&tasks, state.active_tab);
				break;
			case 'h':
			case 68: // LEFT arrow
			case 'Z': // Shift+Tab
				state.active_tab = (state.active_tab == 0) ? TASK_STATUS_COUNT-1 : state.active_tab-1;
				state.selected_index = first_visible(&tasks, state.active_tab);
				break;
			case 'd':
				if (tasks.count > 0 && state.selected_index < tasks.count) {
					task_remove(&tasks, state.selected_index);
					if (state.selected_index >= tasks.count && tasks.count > 0)
						state.selected_index = tasks.count - 1;
					state.selected_index = first_visible(&tasks, state.active_tab);
				}
				break;
			case 'a':
				state.new_task = true;
				break;
			case 'e':
			case 'E':
				if (tasks.count > 0 && state.selected_index < tasks.count) {
					printf("\e[2J\e[H");
					Task* t = &tasks.items[state.selected_index];

					printf("%s%sName: %s", ANSI_FG_BLUE, ANSI_BOLD, ANSI_RESET);
					fflush(stdout);
					if (read_line_inline(t->name, TASK_NAME_BUFFER_SIZE)) {
						printf("%s%sDescription: %s", ANSI_FG_TEAL, ANSI_BOLD, ANSI_RESET);
						fflush(stdout);
						read_line_inline(t->description, TASK_DESC_BUFFER_SIZE);
					}

					printf("\e[?25l");
					fflush(stdout);
				}
				break;
		case SHIFT_UP:
			if (state.selected_index > 0) {
				for (size_t i = state.selected_index - 1; ; --i) {
					if (tasks.items[i].status == state.active_tab) {
						Task tmp = tasks.items[state.selected_index];
						tasks.items[state.selected_index] = tasks.items[i];
						tasks.items[i] = tmp;
						state.selected_index = i;
						break;
					}
					if (i == 0) break;
				}
			}
			break;
		case SHIFT_DOWN:
			if (state.selected_index < tasks.count - 1) {
				for (size_t i = state.selected_index + 1; i < tasks.count; ++i) {
					if (tasks.items[i].status == state.active_tab) {
						Task tmp = tasks.items[state.selected_index];
						tasks.items[state.selected_index] = tasks.items[i];
						tasks.items[i] = tmp;
						state.selected_index = i;
						break;
					}
				}
			}
			break;
			case 'k':
			case 65: // UP
				if (tasks.count > 0) {
					size_t i = state.selected_index;
					size_t seen = 0;
					do {
						i = (i == 0) ? tasks.count - 1 : i - 1;
						seen++;
					} while (seen < tasks.count && tasks.items[i].status != state.active_tab);
					if (tasks.items[i].status == state.active_tab)
						state.selected_index = i;
				}
				break;
			case 'j':
			case 66: // DOWN
				if (tasks.count > 0) {
					size_t i = state.selected_index;
					size_t seen = 0;
					do {
						i = (i + 1) % tasks.count;
						seen++;
					} while (seen < tasks.count && tasks.items[i].status != state.active_tab);
					if (tasks.items[i].status == state.active_tab)
						state.selected_index = i;
				}
				break;
			case 'H':
			case SHIFT_LEFT:
				if (state.active_tab > 0 && state.selected_index < tasks.count) {
					tasks.items[state.selected_index].status = state.active_tab - 1;
					state.active_tab--;
					state.selected_index = first_visible(&tasks, state.active_tab);
				}
				break;
			case 'L':
			case SHIFT_RIGHT:
				if (state.active_tab < TASK_STATUS_COUNT-1 && state.selected_index < tasks.count) {
					tasks.items[state.selected_index].status = state.active_tab + 1;
					state.active_tab++;
					state.selected_index = first_visible(&tasks, state.active_tab);
				}
				break;
		}

		if (state.new_task) {
			printf("\e[2J\e[H");
			state.new_task = false;

			Task new = {0};

			printf("%s%sName: %s", ANSI_FG_BLUE, ANSI_BOLD, ANSI_RESET);
			fflush(stdout);
			if (read_line_inline(new.name, TASK_NAME_BUFFER_SIZE)) {
				printf("%s%sDescription: %s", ANSI_FG_TEAL, ANSI_BOLD, ANSI_RESET);
				fflush(stdout);
				if (read_line_inline(new.description, TASK_DESC_BUFFER_SIZE)) {
					new.status = TASK_TODO;
					task_append(&tasks, new);
					state.selected_index = tasks.count - 1;
				}
			}

			printf("\e[?25l");
			fflush(stdout);
		}
	}
	printf("\e[2J\e[H");
	enable_input_buffering();

	if(!tasks_save(&tasks, (char*)data_file)) {
		fprintf(stderr, "[ERRO] Failed to save %s\n", data_file);
		return 1;
	}
	
	return 0;
}