link_directories(D:/devel/lib64)
include_directories(D:/devel/include)
set(ENV{PATH} D:/devel/bin64$ENV{PATH})

add_executable(app main.cpp draw.cpp task.cpp)
target_link_libraries(app SDL2 opengl32 kernel32)