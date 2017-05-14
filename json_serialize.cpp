
#include "thirdparty/json.hpp"
#include <fstream>

void SerializeTaskList(const TaskList& taskList) {
  nlohmann::json json;
  json["lastAccessTime"] = 0;
  for (size_t i = 0; i < taskList.tasks.size(); i++) {
    auto& task = taskList.tasks[i];
    json["tasks"][i]["name"] = task.name;
    json["tasks"][i]["description"] = task.description;
    json["tasks"][i]["notes"] = task.notes;
    for (size_t j = 0; j < task.taskEvents.size(); j++) {
      auto& event = task.taskEvents[j];
      json["tasks"][i]["events"][j]["type"] = (uint32_t)event.type;
      json["tasks"][i]["events"][j]["time"] = event.timestamp;
      json["tasks"][i]["events"][j]["difficulty"] = (uint32_t)event.difficulty;
    }
  }

  std::ofstream out("tasks.json");
  out << std::setw(2) << json << "\n";
}

void DeserializeTaskList(TaskList& taskList) {
  std::ifstream in("tasks.json");
  nlohmann::json json;
  in >> json;

  uint64_t lastAccessTime = json["lastAccessTime"];
  auto jsonTaskList = json["tasks"];
  for (size_t i = 0; i < jsonTaskList.size(); i++) {
    taskList.tasks.push_back(Task{});
    auto& task = taskList.tasks.back();
    task.name = jsonTaskList[i]["name"].get<std::string>();
    task.description = jsonTaskList[i]["description"].get<std::string>();
    task.notes = jsonTaskList[i]["notes"].get<std::string>();
    auto eventList = jsonTaskList[i]["events"];
    for (size_t j = 0; j < eventList.size(); j++) {
      task.taskEvents.push_back(TaskEvent{});
      auto& event = task.taskEvents.back();
      event.type = (TaskEventType)(eventList[j]["type"].get<uint32_t>());

      event.timestamp = eventList[j]["time"].get<int64_t>();
      event.difficulty = (TaskDifficulty)(eventList[j]["difficulty"].get<uint32_t>());
    }
  }
}