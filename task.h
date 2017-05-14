
#include <stdint.h>
#include <string>
#include <vector>
#include <time.h>

enum class TaskEventType {
  Created,
  Scheduled,
  Completed,
};

static const char *TaskEventTypeStrings[] = {
  "Created",
  "Scheduled",
  "Completed",
};

enum class TaskDifficulty {
  VeryEasy,
  Easy,
  Medium,
  Hard,
  VeryHard,
  COUNT,
};

static const char *TaskDifficultyName[]{
  "VeryEasy",
  "Easy",
  "Medium",
  "Hard",
  "VeryHard"
};

struct TaskEvent {
  TaskEventType type;
  int64_t timestamp;
  TaskDifficulty difficulty;
};

struct Task {
  std::string name;
  std::string description;
  std::string notes;
  std::vector<TaskEvent> taskEvents;
  bool wasUnscheduledThisRun;

  void AddEvent(TaskEventType type) {
    int64_t timestamp = time(0);
    TaskDifficulty difficulty = TaskDifficulty::Medium;
    if (taskEvents.size() > 0) {
      auto& event = taskEvents.back();
      difficulty = event.difficulty;
    }

    taskEvents.emplace_back(TaskEvent{ type, timestamp, difficulty });
  }

};

struct TaskList {
  std::vector<Task> tasks;
  uint64_t CreateTask();
  void AutoSchedule();
};

static const uint64_t INVALID_TASK_ID = 0xFFFFFFFFFFFFFFFF;

