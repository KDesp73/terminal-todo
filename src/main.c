#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "todo.h"

typedef struct {
	size_t active_tab;
	size_t selected_index;
	bool new_task;
	bool running;
} UIState;

bool task_form(Task* task)
{
	printf("%s%sName:%s ", ANSI_FG_BLUE, ANSI_BOLD, ANSI_RESET);
	if (fgets(task->name, sizeof(task->name), stdin) == NULL) return false;
	strip_newline(task->name);
	printf("%s%sDescription:%s ", ANSI_FG_TEAL, ANSI_BOLD, ANSI_RESET);
	if (fgets(task->description, sizeof(task->description), stdin) == NULL) return false;
	strip_newline(task->description);

	task->priority = 0;
	task->status = TASK_TODO;

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
	putchar('\n');

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
	for (int i = 0; i < 38; ++i) printf("─");
	printf("%s\n", ANSI_RESET);
	printf("  ");
	printf("%s[q]%s quit  %s[a]%s add  %s[j/k]%s nav  %s[h/l]%s move  %s[tab]%s switch%s\n",
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

static int task_cmp(const void* a, const void* b)
{
	const Task* ta = (const Task*)a;
	const Task* tb = (const Task*)b;
	int d = (int)ta->status - (int)tb->status;
	if (d) return d;
	return (int)ta->priority - (int)tb->priority;
}

#define SHIFT_UP   165
#define SHIFT_DOWN 166

static int read_key(void)
{
	int c = getchar();
	if (c != '\e') return c;

	c = getchar();
	if (c != '[') return '\e';

	c = getchar();
	if (c >= 'A' && c <= 'D') return c;
	if (c == 'Z') return 'Z';

	int p1 = 0;
	while (c >= '0' && c <= '9') {
		p1 = p1 * 10 + (c - '0');
		c = getchar();
	}

	if (c == ';') {
		int p2 = 0;
		c = getchar();
		while (c >= '0' && c <= '9') {
			p2 = p2 * 10 + (c - '0');
			c = getchar();
		}
		if (p1 == 1 && p2 == 2) {
			if (c == 'A') return SHIFT_UP;
			if (c == 'B') return SHIFT_DOWN;
		}
		return c;
	}

	return c;
}

int main(int argc, char** argv)
{
	Tasks tasks = {0};
	if(!tasks_load(&tasks, DEFAULT_FILE)) {
		fprintf(stderr, "[WARN] Could not open %s file\n", DEFAULT_FILE);
	}

	qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);

	for (size_t i = 0; i < tasks.count; ++i) {
		size_t p = 0;
		for (size_t j = 0; j < i; ++j)
			if (tasks.items[j].status == tasks.items[i].status) p++;
		tasks.items[i].priority = p;
	}

	UIState state = {.running = true};
	state.selected_index = first_visible(&tasks, state.active_tab);

	disable_input_buffering();


	while (state.running) {		
		printf("\e[2J\e[H");
		tasks_table(&tasks, &state);

		int key = read_key();
		switch (key) {
			case 'q': 
				state.running = false;
				break;
			case '\t':
				state.active_tab = (state.active_tab+1) % TASK_STATUS_COUNT;
				state.selected_index = first_visible(&tasks, state.active_tab);
				break;
			case 'Z': // Shift+Tab
				state.active_tab = (state.active_tab == 0) ? TASK_STATUS_COUNT-1 : state.active_tab-1;
				state.selected_index = first_visible(&tasks, state.active_tab);
				break;
			case 'a':
				state.new_task = true;
				break;
			case SHIFT_UP:
				if (tasks.count > 0 && state.selected_index < tasks.count) {
					size_t above = state.selected_index;
					while (above > 0) {
						above--;
						if (tasks.items[above].status == state.active_tab) break;
					}
					if (above != state.selected_index && tasks.items[above].status == state.active_tab) {
						char name[TASK_NAME_BUFFER_SIZE];
						strncpy(name, tasks.items[state.selected_index].name, TASK_NAME_BUFFER_SIZE);
						name[TASK_NAME_BUFFER_SIZE - 1] = '\0';
						size_t tmp = tasks.items[state.selected_index].priority;
						tasks.items[state.selected_index].priority = tasks.items[above].priority;
						tasks.items[above].priority = tmp;
						qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);
						for (size_t i = 0; i < tasks.count; ++i)
							if (strcmp(tasks.items[i].name, name) == 0) {
								state.selected_index = i;
								break;
							}
					}
				}
				break;
			case SHIFT_DOWN:
				if (tasks.count > 0 && state.selected_index < tasks.count) {
					size_t below = state.selected_index;
					while (below < tasks.count - 1) {
						below++;
						if (tasks.items[below].status == state.active_tab) break;
					}
					if (below != state.selected_index && tasks.items[below].status == state.active_tab) {
						char name[TASK_NAME_BUFFER_SIZE];
						strncpy(name, tasks.items[state.selected_index].name, TASK_NAME_BUFFER_SIZE);
						name[TASK_NAME_BUFFER_SIZE - 1] = '\0';
						size_t tmp = tasks.items[state.selected_index].priority;
						tasks.items[state.selected_index].priority = tasks.items[below].priority;
						tasks.items[below].priority = tmp;
						qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);
						for (size_t i = 0; i < tasks.count; ++i)
							if (strcmp(tasks.items[i].name, name) == 0) {
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
			case 'h':
			case 68: // LEFT arrow
				if (state.active_tab > 0) {
					state.active_tab--;
					if (state.selected_index < tasks.count)
						tasks.items[state.selected_index].status = state.active_tab;
					qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);
					state.selected_index = first_visible(&tasks, state.active_tab);
				}
				break;
			case 'l':
			case 67: // RIGHT arrow
				if (state.active_tab < TASK_STATUS_COUNT-1) {
					state.active_tab++;
					if (state.selected_index < tasks.count)
						tasks.items[state.selected_index].status = state.active_tab;
					qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);
					state.selected_index = first_visible(&tasks, state.active_tab);
				}
				break;
		}

		if (state.new_task) {
			printf("\e[2J\e[H");
			enable_input_buffering();

			state.new_task = false;

			Task new = {0};
			task_form(&new);
			task_append(&tasks, new);
			qsort(tasks.items, tasks.count, sizeof(Task), task_cmp);

			disable_input_buffering();
		}
	}
	printf("\e[2J\e[H");
	enable_input_buffering();

	if(!tasks_save(&tasks, DEFAULT_FILE)) {
		fprintf(stderr, "[ERRO] Failed to save %s file\n", DEFAULT_FILE);
		return 1;
	}
	
	return 0;
}