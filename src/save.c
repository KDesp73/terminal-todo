#include "todo.h"
#include <stdlib.h>

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

	char line[4 + 2 + 2 + 1 + TASK_NAME_BUFFER_SIZE + 2 + TASK_DESC_BUFFER_SIZE + 1];
	size_t lineno = 0;
	while (fgets(line, sizeof(line), fd)) {
		lineno++;
		strip_newline(line);

		Task task = {0};

		char tag[5] = {0};
		if (sscanf(line, "%4s", tag) != 1) {
			fprintf(stderr, "[WARN] %s:%zu: empty line, skipping\n", file, lineno);
			continue;
		}

		if      (strcmp(tag, "TODO") == 0) task.status = TASK_TODO;
		else if (strcmp(tag, "PROG") == 0) task.status = TASK_IN_PROGRESS;
		else if (strcmp(tag, "TEST") == 0) task.status = TASK_TESTING;
		else if (strcmp(tag, "DONE") == 0) task.status = TASK_DONE;
		else {
			fprintf(stderr, "[WARN] %s:%zu: unknown tag \"%s\", expected TODO/PROG/TEST/DONE\n         → %s\n", file, lineno, tag, line);
			continue;
		}

		char* paren_start = strchr(line, '(');

		if (paren_start && paren_start == line + 4) {
			// New format: TAG(p): name [desc]
			char* paren_end = strchr(line, ')');
			if (!paren_end || paren_end <= paren_start) {
				fprintf(stderr, "[WARN] %s:%zu: malformed \"(priority)\"\n         → %s\n", file, lineno, line);
				continue;
			}

			char priority_str[16];
			size_t p_len = paren_end - paren_start - 1;
			if (p_len >= sizeof(priority_str)) p_len = sizeof(priority_str) - 1;
			strncpy(priority_str, paren_start + 1, p_len);
			priority_str[p_len] = '\0';
			task.priority = (size_t)atol(priority_str);

			char* name_start = paren_end + 2;
			while (*name_start == ' ') name_start++;
			char* bracket_start = strstr(name_start, " [");
			if (bracket_start) {
				size_t name_len = bracket_start - name_start;
				strncpy(task.name, name_start, name_len);
				task.name[name_len] = '\0';

				char* desc_start = bracket_start + 2;
				char* desc_end = strchr(desc_start, ']');
				if (desc_end) {
					size_t desc_len = desc_end - desc_start;
					strncpy(task.description, desc_start, desc_len);
					task.description[desc_len] = '\0';
				} else {
					fprintf(stderr, "[WARN] %s:%zu: missing closing \"]\" for description\n         → %s\n         description will be empty\n", file, lineno, line);
				}
			} else {
				strncpy(task.name, name_start, TASK_NAME_BUFFER_SIZE - 1);
				task.name[TASK_NAME_BUFFER_SIZE - 1] = '\0';
			}
		} else {
			// Old format: TAG: name
			char* name = strstr(line, ": ");
			if (!name) {
				fprintf(stderr, "[WARN] %s:%zu: expected \": \" after tag\n         → %s\n", file, lineno, line);
				continue;
			}
			name += 2;
			strncpy(task.name, name, TASK_NAME_BUFFER_SIZE - 1);
			task.name[TASK_NAME_BUFFER_SIZE - 1] = '\0';
		}

		task_append(tasks, task);
	}

	return !fclose(fd);
}
