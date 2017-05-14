
#include "thirdparty/prototype_serializer.h"
#include "task.h"

void SerializeTaskEvent(TaskEvent& event) {
  ps_begin_block("event");
  ps_uint32("type", &event.type);
  ps_uint64("time", &event.timestamp);
  ps_uint32("difficulty", &event.difficulty);
  ps_end_block();
}

void SerializeString(const char *name, std::string& s) {
  ps_begin_block(name);
  uint32_t length = s.size();
  ps_uint32("length", &length);
  if (ps_is_reading()) {
    s.resize(length);
  }
  ps_uint8_buffer("data", length, s.data());
  ps_end_block();
}

void SerializeTask(Task& task) {
  ps_begin_block("task");
  SerializeString("name", &task.name);
  SerializeString("description", &task.description);
  SerializeString("notes", &task.notes);
  uint64_t eventCount = task.taskEvents.size();
  ps_string("event_count", &eventCount); //issue
  if (ps_is_reading()) {
    task.taskEvents.resize(eventCount);
  }

  ps_begin_block_array("events");
  for (size_t i = 0; i < task.taskEvents.size(); i++) {
    SerializeTaskEvent(task.taskEvents[i]);
  }
  ps_end_block_array();
  ps_end_block();
}

void SerializeHeader(TaskList& taskList) {
  ps_begin_block("header");
  uint64_t taskCount = taskList.tasks.size();
  ps_uint64("taskCount", taskCount);
  if (ps_is_reading()) {
    taskList.tasks.resize(taskCount);
  }
  ps_end_block();
}

void SerializeTaskFile(TaskList& taskList, PSMode mode) {
  ps_open_file("tasks.ps", mode);
  SerializeHeader(taskList);
  for (size_t i = 0; i < taskList.tasks.size(); i++) {
    SerializeTask(taskList.tasks[i]);
  }
  ps_close_file();
}