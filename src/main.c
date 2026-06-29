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

int main(int argc, char** argv)
{
	Tasks tasks = {0};
	if(!tasks_load(&tasks, DEFAULT_FILE)) {
		fprintf(stderr, "[WARN] Could not open %s file\n", DEFAULT_FILE);
	}

	UIState state = {.running = true};

	disable_input_buffering();


	while (state.running) {		
		printf("\e[2J\e[H");
		tasks_table(&tasks, &state);

		int key = getchar();
		switch (key) {
			case 'q': 
				state.running = false;
				break;
			case '\t':
				state.active_tab = (state.active_tab+1) % TASK_STATUS_COUNT;
				state.selected_index = 0;
				break;
			case 'a':
				state.new_task = true;
				break;
			case 'k':
			case 65: // UP
				state.selected_index = (state.selected_index == 0) ? tasks.count-1 : state.selected_index-1;
				break;
			case 'j':
			case 66: // DOWN
				state.selected_index = (state.selected_index == tasks.count-1) ? 0 : state.selected_index+1;
				break;
			case 'h':
			case 68: // LEFT arrow
				if (state.active_tab > 0) {
					state.active_tab--;
					if (state.selected_index < tasks.count) {
						tasks.items[state.selected_index].status = state.active_tab;
					}
					state.selected_index = 0;
				}
				break;
			case 'l':
			case 67: // RIGHT arrow
				if (state.active_tab < TASK_STATUS_COUNT-1) {
					state.active_tab++;
					if (state.selected_index < tasks.count) {
						tasks.items[state.selected_index].status = state.active_tab;
					}
					state.selected_index = 0;
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