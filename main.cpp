#define QUICKAPP_IMGUI
#define QUICKAPP_IMPLEMENTATION
#define QUICKAPP_IMGUI_FONT_FILE "Blogger Sans.ttf"
#define QUICKAPP_IMGUI_FONT_SIZE 16

#include "QuickApp/QuickApp.h"

#include <stdint.h>

#include "task.h"

#include <string.h>

#include "json_serialize.cpp"

static const uint64_t INVALID_UINT64 = 0xFFFFFFFFFFFFFFFF;

#define ImRGB8(r, g, b, a) ImVec4((float)r / 255, (float)g / 255, (float)b / 255, 1.0f)

enum SortType {
  SortType_None,
  SortType_Type,
  SortType_Name,
  SortType_Date,
};

void DrawYearMonthDay(int64_t timestamp) {
  struct tm timeStruct = *localtime(&timestamp);
  ImGui::Text("%u-%.2u-%.2u", (uint32_t)(timeStruct.tm_year + 1900), (uint32_t)timeStruct.tm_mon + 1, (uint32_t)timeStruct.tm_mday);
}

class Editor {
  uint64_t selectedTaskID;
  uint64_t selectedHistoryID;

  //TODO(Torin) Make these dynamic just in case
  char descriptionBuffer[1024 * 1024];
  char notesBuffer[1024 * 1024];
  char taskNameBuffer[1 << 16];

  bool taskWasJustSelected = false;
  bool isWindowDragging = false;
  bool isFirstDrawCall = true;
  TaskDifficulty radioButtonDifficulty = TaskDifficulty::Medium;

  //Used to sort and display tasks in order
  SortType currentSortType = SortType_None;
  bool sortAsscendingToggle = false;
  std::vector<uint64_t> tasksToList;

public:
  uint32_t windowWidth = 1600;
  uint32_t windowHeight = 900;
  uint32_t titlebarHeight = 20;
  uint32_t borderSize = 4;
  uint32_t sidebarSize = (windowWidth - (borderSize * 2)) / 3;

  int mainSeperatorOffset = 600;
  int statusColumnDefaultWidth = 45;
  int dateColumnDefaultWidth = 45;

  //ImGui Style Colors
  ImVec4 titlebarColor = ImRGB8(45, 45, 48);
  ImVec4 backgroundColor = ImRGB8(12, 12, 12);
  ImVec4 buttonColor = ImRGB8(45, 45, 48);
  ImVec4 activeColor = ImRGB8(65, 65, 65);

  Editor(const TaskList& taskList) {
    selectedTaskID = INVALID_TASK_ID;
    memset(descriptionBuffer, 0x00, sizeof(descriptionBuffer));
    memset(taskNameBuffer, 0x00, sizeof(taskNameBuffer));
    memset(notesBuffer, 0x00, sizeof(notesBuffer));
    tasksToList.reserve(taskList.tasks.size());
    SortTasksToList(taskList, SortType_Date);
  }

  void SetSelectedTask(uint64_t taskID, TaskList& taskList) {
    selectedTaskID = taskID;
    selectedHistoryID = INVALID_UINT64;
    auto& task = taskList.tasks[taskID];
    assert(task.description.length() < sizeof(descriptionBuffer));
    assert(task.name.length() < sizeof(taskNameBuffer));
    memset(descriptionBuffer, 0x00, sizeof(descriptionBuffer));
    memset(taskNameBuffer, 0x00, sizeof(taskNameBuffer));
    memset(notesBuffer, 0x00, sizeof(notesBuffer));
    memcpy(taskNameBuffer, task.name.c_str(), task.name.length());
    memcpy(descriptionBuffer, task.description.c_str(), task.description.length());
    memcpy(notesBuffer, task.notes.c_str(), task.notes.length());
    taskWasJustSelected = true;
  }

  void SortTasksToList(const TaskList& taskList, SortType sortType) {
    assert(sortType != SortType_None);

    if (currentSortType == sortType) {
      sortAsscendingToggle = !sortAsscendingToggle;
    } else {
      currentSortType = sortType;
      sortAsscendingToggle = false;
    }

    tasksToList.clear();
    for (size_t i = 0; i < taskList.tasks.size(); i++) {
      const Task& task = taskList.tasks[i];
      tasksToList.push_back(i);
    }

    std::sort(tasksToList.begin(), tasksToList.end(), [&](uint64_t a, uint64_t b) {
      const Task& taskA = taskList.tasks[a];
      const Task& taskB = taskList.tasks[b];
      if (sortType == SortType_Name) {
        return sortAsscendingToggle ? taskA.name > taskB.name : taskA.name < taskB.name;
      }

      const TaskEvent& eventA = taskA.taskEvents.back();
      const TaskEvent& eventB = taskB.taskEvents.back();
      if (sortType == SortType_Type) {
        return sortAsscendingToggle ? eventA.type > eventB.type : eventA.type < eventB.type;
      } else if (sortType == SortType_Date) {
        return sortAsscendingToggle ? eventA.timestamp > eventB.timestamp : eventA.timestamp < eventB.timestamp;
      }
    });
  }

  void DrawTaskListEntries(uint64_t *taskIDs, size_t count, TaskList& taskList) {
    ImGui::Columns(3, "TaskEntryInfo");
    if (isFirstDrawCall) {
      ImGui::SetColumnOffset(1, 375);
      ImGui::SetColumnOffset(2, 475);
    }

    if (ImGui::Selectable("Name", false)) {
      SortTasksToList(taskList, SortType_Name);
    }

    ImGui::NextColumn();
    if (ImGui::Selectable("Date", false)) {
      SortTasksToList(taskList, SortType_Date);
    }
    ImGui::NextColumn();
    if (ImGui::Selectable("Status", false)) {
      SortTasksToList(taskList, SortType_Type);
    }

    ImGui::NextColumn();
    ImGui::Separator();

    for (size_t i = 0; i < count; i++) {
      const Task& task = taskList.tasks[taskIDs[i]];
      if (ImGui::Selectable(task.name.c_str(), selectedTaskID == taskIDs[i])) {
        SetSelectedTask(taskIDs[i], taskList);
      }

      ImGui::NextColumn();
      const TaskEvent& mostRecentEvent = task.taskEvents.back();
      DrawYearMonthDay(mostRecentEvent.timestamp);
      ImGui::NextColumn();
      const char *eventTypeString = TaskEventTypeStrings[(uint32_t)mostRecentEvent.type];
      ImGui::Text(eventTypeString);
      ImGui::NextColumn();
    }
  }

  void DrawTaskInfo(Task& task) {
    ImGui::PushItemWidth(windowWidth - sidebarSize);
    ImGui::PushID(0);
    if (ImGui::InputText("", taskNameBuffer, sizeof(taskNameBuffer))) {
      task.name.assign(taskNameBuffer);
    }
    ImGui::PopID();

    ImGui::PushID(1);
    if (ImGui::InputTextMultiline("", (char *)descriptionBuffer, sizeof(descriptionBuffer))) {
      task.description.assign((char *)descriptionBuffer);
    }
    ImGui::PopID();

    if (taskWasJustSelected) {
      ImGui::SetNextTreeNodeOpen(false);
      taskWasJustSelected = false;
    }



    if (ImGui::CollapsingHeader("Spoiler Notes")) {
      ImGui::PushID(2);
      if (ImGui::InputTextMultiline("", (char *)notesBuffer, sizeof(notesBuffer))) {
        task.notes.assign((char *)notesBuffer);
      }
      ImGui::PopID();
    }

    if (taskWasJustSelected) {
      ImGui::SetNextTreeNodeOpen(true);
    }

    if (ImGui::CollapsingHeader("Task Events")) {
      ImGui::PopItemWidth();
      ImGui::Text("History");
      ImGui::PushStyleColor(ImGuiCol_Header, activeColor);
      ImGui::BeginChildFrame(0, ImVec2(400, 200));
      ImGui::Columns(3);
      for (size_t i = 0; i < task.taskEvents.size(); i++) {
        TaskEvent& event = task.taskEvents[i];
        ImGui::PushID(i);
        if (ImGui::Selectable(TaskEventTypeStrings[(size_t)event.type], selectedHistoryID == i, ImGuiSelectableFlags_SpanAllColumns)) {
          selectedHistoryID = i;
        }
        ImGui::NextColumn();
        DrawYearMonthDay(event.timestamp);
        ImGui::NextColumn();
        ImGui::Text("%s", TaskDifficultyName[(size_t)event.difficulty]);
        ImGui::NextColumn();
        ImGui::PopID();
      }
      ImGui::EndChildFrame();
      ImGui::PopStyleColor();

      if (ImGui::Button("Delete Event")) {
        //NOTE(Torin) Can't delete creation event(selectedHistoryID == 0)
        if (selectedHistoryID != INVALID_UINT64 && selectedHistoryID != 0) {
          task.taskEvents.erase(task.taskEvents.begin() + selectedHistoryID);
          selectedHistoryID = INVALID_UINT64;
        }
      }


      ImGui::SameLine();
      assert(task.taskEvents.size() >= 1); //Must always have creation event
      TaskEvent& lastEvent = task.taskEvents[task.taskEvents.size() - 1];
      if (lastEvent.type == TaskEventType::Scheduled) {
        if (ImGui::Button("Complete")) {
          task.AddEvent(TaskEventType::Completed);
          auto& event = task.taskEvents.back();
          event.difficulty = radioButtonDifficulty;
        }
      } else {
        if (ImGui::Button("Schedule")) {
          task.AddEvent(TaskEventType::Scheduled);
          auto& event = task.taskEvents.back();
          event.difficulty = radioButtonDifficulty;
        }
      }



      for (size_t i = 0; i < (size_t)TaskDifficulty::COUNT; i++) {
        ImGui::RadioButton(TaskDifficultyName[i], (int *)&radioButtonDifficulty, i);
      }

    } else {
      ImGui::PopItemWidth();
    }
  }





  void DrawTaskButtons(TaskList& taskList) {
    if (ImGui::Button("Add Task")) {
      taskList.CreateTask();
      //This is a hack to perserve the accendingToogle because SortTasksToList will flip it
      sortAsscendingToggle = !sortAsscendingToggle;
      SortTasksToList(taskList, currentSortType);
    }

    ImGui::SameLine();

    if (ImGui::Button("Schedule")) {

    }

    ImGui::SameLine();

    if (ImGui::Button("Save")) {
      SerializeTaskList(taskList);
    }
  }


  void DrawTitlebarButtons() {
    static const uint32_t imguiWindowFlags = ImGuiWindowFlags_NoMove |
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | 
      ImGuiWindowFlags_NoScrollbar;
    static const uint32_t buttonWindowWidth = 24;
    ImGui::SetNextWindowSize(ImVec2(buttonWindowWidth, titlebarHeight));
    ImGui::SetNextWindowPos(ImVec2(windowWidth - buttonWindowWidth, 0.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, titlebarColor);
    ImGui::Begin("TitlebarButtons", 0, imguiWindowFlags);
    if (ImGui::Button("x")) {
      QuickApp::Internal::isRunning = false;
    }
    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
  }


  void DrawTaskListPanel(TaskList& taskList) {
    ImGui::BeginChild("Tasks");
    DrawTaskButtons(taskList);
    ImGui::Separator();
    DrawTaskListEntries(tasksToList.data(), tasksToList.size(), taskList);
    ImGui::EndChild();
  }

  void Draw(TaskList& taskList) {
    uint32_t imguiWindowFlags =  ImGuiWindowFlags_NoMove | 
      ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, titlebarColor);
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth, (float)windowHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));

    ImGui::Begin("TitlebarAndBorder", 0, imguiWindowFlags | ImGuiWindowFlags_NoInputs);
    if (QuickApp::Input::localMouseY < titlebarHeight && ImGui::IsMouseDown(0)) {
      isWindowDragging = true;
    }

    if (isWindowDragging) {
      if (ImGui::IsMouseDown(0) == false) {
        isWindowDragging = false;
      } else {
        int windowX, windowY;
        SDL_GetWindowPosition(QuickApp::Internal::window, &windowX, &windowY);
        windowX += QuickApp::Input::deltaMouseX;
        windowY += QuickApp::Input::deltaMouseY;
        SDL_SetWindowPosition(QuickApp::Internal::window, windowX, windowY);
      }
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();

    ImGui::SetNextWindowFocus();
    ImGui::SetNextWindowSize(ImVec2((float)windowWidth - (borderSize * 2), (float)(windowHeight - titlebarHeight - borderSize)));
    ImGui::SetNextWindowPos(ImVec2(borderSize, titlebarHeight));
    
    ImGui::Begin("Tasks", 0, imguiWindowFlags);
    ImGui::Columns(2);
    static bool isFirstDraw = true;
    if (isFirstDraw) {
      ImGui::SetColumnOffset(1, mainSeperatorOffset);
      isFirstDraw = false;
    }


   


    DrawTaskListPanel(taskList);
    ImGui::NextColumn();

    if (selectedTaskID != INVALID_TASK_ID) {
      DrawTaskInfo(taskList.tasks[selectedTaskID]);
    }
    
    ImGui::End();
    DrawTitlebarButtons();

    isFirstDrawCall = false;
  }
};


int main() {
  TaskList taskList;
  DeserializeTaskList(taskList);

  Editor *editor = new Editor(taskList);


  { //Set the default ImGui Style
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.WindowMinSize = ImVec2(0.0f, 0.0f);
    style.Colors[ImGuiCol_WindowBg] = editor->backgroundColor;
    style.Colors[ImGuiCol_Button] = editor->buttonColor;
    style.Colors[ImGuiCol_Header] = editor->buttonColor;
    style.Colors[ImGuiCol_ButtonHovered] = editor->activeColor;
    style.Colors[ImGuiCol_HeaderHovered] = editor->activeColor;
    style.Colors[ImGuiCol_ButtonActive] = editor->activeColor;
    style.Colors[ImGuiCol_HeaderActive] = editor->activeColor;
    style.Colors[ImGuiCol_FrameBg] = editor->buttonColor;
    style.Colors[ImGuiCol_ScrollbarBg] = editor->buttonColor;
    style.Colors[ImGuiCol_ScrollbarGrab] = editor->activeColor;
  }

  QuickApp::Start("Practicar", editor->windowWidth, editor->windowHeight);
  QuickApp::Loop([&]() {
    editor->Draw(taskList);

#if 0
    ImGui::SetNextWindowFocus();
    ImGui::ShowTestWindow();
#endif

  });

}