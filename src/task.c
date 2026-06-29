#include "todo.h"

void status_tag(TaskStatus status, char buffer[])
{
	switch (status) {
	case TASK_TODO:
		strncpy(buffer, "TODO", 5);
		return;
	case TASK_IN_PROGRESS:
		strncpy(buffer, "PROG", 5);
		return;
	case TASK_TESTING:
		strncpy(buffer, "TEST", 5);
		return;
	case TASK_DONE:
		strncpy(buffer, "DONE", 5);
		return;
	case TASK_STATUS_COUNT:
		return;
	}
}

bool task_append(Tasks* tasks, Task task)
{
	if (tasks->count >= MAX_TASKS) return false;
	tasks->items[tasks->count++] = task;
	return true;
}

bool task_remove(Tasks* tasks, size_t index)
{
	if (index >= tasks->count) return false;
	for (size_t i = index; i < tasks->count-1; ++i) {
		tasks->items[i] = tasks->items[i+1];
	}
	tasks->count--;
	return true;
}

int task_print(Task task, FILE* fd)
{
	char tag[5];
	status_tag(task.status, tag);
	return fprintf(fd, "%s(%2zu): %s [%s]\n", tag, task.priority, task.name, task.description);
}