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
	while (fgets(line, sizeof(line), fd)) {
		strip_newline(line);

		Task task = {0};

		char tag[5] = {0};
		if (sscanf(line, "%4s", tag) != 1) continue;

		if      (strcmp(tag, "TODO") == 0) task.status = TASK_TODO;
		else if (strcmp(tag, "PROG") == 0) task.status = TASK_IN_PROGRESS;
		else if (strcmp(tag, "TEST") == 0) task.status = TASK_TESTING;
		else if (strcmp(tag, "DONE") == 0) task.status = TASK_DONE;
		else                               continue;

		char* paren_start = strchr(line, '(');
		char* paren_end   = strchr(line, ')');
		if (!paren_start || !paren_end || paren_end <= paren_start) continue;

		char priority_str[16];
		size_t p_len = paren_end - paren_start - 1;
		strncpy(priority_str, paren_start + 1, p_len);
		priority_str[p_len] = '\0';
		task.priority = (size_t)atol(priority_str);

		char* name_start = paren_end + 2;
		char* bracket_start = strstr(name_start, " [");
		if (!bracket_start) continue;

		size_t name_len = bracket_start - name_start;
		strncpy(task.name, name_start, name_len);
		task.name[name_len] = '\0';

		char* desc_start = bracket_start + 2;
		char* desc_end = strchr(desc_start, ']');
		if (desc_end) {
			size_t desc_len = desc_end - desc_start;
			strncpy(task.description, desc_start, desc_len);
			task.description[desc_len] = '\0';
		}

		task_append(tasks, task);
	}

	return !fclose(fd);
}
