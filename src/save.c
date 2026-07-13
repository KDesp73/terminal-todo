#include "todo.h"

bool tasks_save(Tasks* tasks, char* file)
{
	FILE* fd = fopen(file, "w");

	for (size_t i = 0; i < tasks->count; ++i) {
		task_print(tasks->items[i], fd);
	}

	return !fclose(fd);
}

bool tasks_load(Tasks* tasks, char* file)
{
	FILE* fd = fopen(file, "r");
	if (!fd) return false;

	bool has_errors = false;
	char line[4 + 2 + 2 + 1 + TASK_NAME_BUFFER_SIZE + 2 + TASK_DESC_BUFFER_SIZE + 1];
	size_t lineno = 0;
	while (fgets(line, sizeof(line), fd)) {
		lineno++;
		strip_newline(line);

		Task task = {0};

		char tag[5] = {0};
		if (sscanf(line, "%4s", tag) != 1) {
			continue;
		}

		if      (strcmp(tag, "TODO") == 0) task.status = TASK_TODO;
		else if (strcmp(tag, "PROG") == 0) task.status = TASK_IN_PROGRESS;
		else if (strcmp(tag, "TEST") == 0) task.status = TASK_TESTING;
		else if (strcmp(tag, "DONE") == 0) task.status = TASK_DONE;
		else {
			fprintf(stderr, "[WARN] %s:%zu: unknown tag \"%s\", expected TODO/PROG/TEST/DONE\n         → %s\n", file, lineno, tag, line);
			has_errors = true;
			continue;
		}

		// Find content after tag — skip optional (priority) if present (legacy)
		char* content = line + 4;
		if (*content == '(') {
			content++;
			while (*content >= '0' && *content <= '9') content++;
			if (*content == ')') content++;
		}
		while (*content == ' ') content++;
		if (*content != ':') {
			fprintf(stderr, "[WARN] %s:%zu: expected \":\" after tag\n         → %s\n", file, lineno, line);
			has_errors = true;
			continue;
		}
		content++;
		while (*content == ' ') content++;

		// Parse name and optional [description]
		char* bracket_start = strstr(content, " [");
		if (bracket_start) {
			size_t name_len = bracket_start - content;
			strncpy(task.name, content, name_len);
			task.name[name_len] = '\0';

			char* desc_start = bracket_start + 2;
			char* desc_end = strchr(desc_start, ']');
			if (desc_end) {
				size_t desc_len = desc_end - desc_start;
				strncpy(task.description, desc_start, desc_len);
				task.description[desc_len] = '\0';
			} else {
				size_t col = desc_start - line;
				fprintf(stderr, "[ERRO] %s:%zu: missing closing \"]\" for description\n         → %s\n", file, lineno, line);
				for (size_t i = 0; i < col + 9; i++) fprintf(stderr, " ");
				fprintf(stderr, "^\n");
				has_errors = true;
				continue;
			}
		} else {
			strncpy(task.name, content, TASK_NAME_BUFFER_SIZE - 1);
			task.name[TASK_NAME_BUFFER_SIZE - 1] = '\0';
		}

		task_append(tasks, task);
	}

	fclose(fd);
	return !has_errors;
}
