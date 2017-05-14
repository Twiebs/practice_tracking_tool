
#include <stdint.h>
#include "task.h"

static const uint64_t TASK_LIST_HEADER_SIGNATURE = 0x4004004004;

struct TaskListHeader {
  uint64_t signature;
  uint64_t taskCount;
} __attribute((packed));

struct TaskHeader {
  DateAndTime creationTime;
  DateAndTime scheduledTime;
  DateAndTime completedTime;
  uint64_t nameSize;
} __attribute((packed));

bool SerializeTaskList(const char *filename, const TaskList& taskList);

//====================================================================================

bool SerializeTaskList(const char *filename, const TaskList& taskList) {
  FILE *file = fopen(filename, "wb");
  if (file == nullptr) return false;
  TaskListHeader fileHeader = {};
  fileHeader.signature = TASK_LIST_HEADER_SIGNATURE;
  fileHeader.taskCount = taskList.tasks.size();
  fwrite(&fileHeader, sizeof(header), 1, file);

  for (size_t i = 0 ; i < header.taskCount; i++) {
    Task& task = taskList.tasks[i];
    TaskHeader taskHeader;
    taskHeader.creationTime = task.creationTime;
    taskHeader.scheduledTime = task.scheduledTime;
    taskHeader.completedTime = task.completedTime;
    taskHeader.nameSize = task.name.size();
    fwrite(&taskHeader, sizeof(TaskHeader), 1, file);
    fwrite(task.name.data(), 1, task.name.size() + 1);
  }
  fclose(file);
}

bool DeserializeTaskList(const char *filename, TaskList& taskList) {
  FILE *file = fopen(filename, "wb");
  if (file == nullptr) return false;
  TaskListHeader fileHeader = {};
  fread(&fileHeader, sizeof(header), 1, file);
  if (fileHeader.signature != TASK_LIST_HEADER_SIGNATURE) {
    return false;
  } 

  for (size_t i = 0; i < fileHeader.taskCount; i++) {
    TaskHeader taskHeader;
    fread(&taskHeader, sizeof(TaskHeader), 1, file); 
    uint64_t taskID = taskList.CreateTask();
    Task& task = taskList.tasks[taskID];
    task.creationTime = taskHeader.creationTime;
    task.scheduledTime = taskHeader.scheduledTime;
    task.completedTime = taskHeader.completedTime;

  }
}