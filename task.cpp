
#include <ctime>
#include "task.h"

uint64_t TaskList::CreateTask() {
  uint64_t taskID = tasks.size();
  tasks.push_back(Task {});
  Task& task = tasks[taskID];
  task.AddEvent(TaskEventType::Created);
  task.name = "Task" + std::to_string(taskID);
  return taskID;
}

void TaskList::AutoSchedule() {
  int64_t currentTime = time(0);

  struct ScheduleScore {
    uint64_t taskID;
    uint64_t score;
  };

  std::vector<ScheduleScore> scores;
  scores.resize(tasks.size());
  for (size_t i = 0; i < scores.size(); i++) {
    auto& task = tasks[i];
    scores[i].taskID = i;
    scores[i].score = 0;

    if (task.taskEvents.back().type == TaskEventType::Scheduled) continue;
    if (task.wasUnscheduledThisRun == true) continue;

    uint32_t timesCompleted = 0;
    for (size_t j = 0; j < task.taskEvents.size(); j++) {
      if (task.taskEvents[j].type == TaskEventType::Completed) {
        timesCompleted += 1;
      }
    }



  }

}