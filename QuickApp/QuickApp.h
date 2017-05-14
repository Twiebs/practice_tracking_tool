/*

Documentation:

Usage:
#define QUICKAPP_IMGUI
#define QUICKAPP_IMPLEMENTATION

Configuration:
#define QUICKAPP_IMGUI_FONT "myFontFile.ttf"

int Start(const char *title, int width, int height);
void Loop(std::function<void()> loopProcedure, std::function<void(SDL_Event *)> = [](SDL_Event *a) {});

*/

#ifndef QUICKAPP_INCLUDE_GUARD
#define QUICKAPP_INCLUDE_GUARD
#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_opengl.h>
#include <functional>

namespace QuickApp {
int Start(const char *title, int width, int height);
void Loop(std::function<void()> loopProcedure, std::function<void(SDL_Event *)> = [](SDL_Event *a) {});
};


#endif//QUICKAPP_INCLUDE_GUARD


#ifdef QUICKAPP_IMPLEMENTATION
#ifdef QUICKAPP_IMGUI
#include "imgui.cpp"
#include "imgui_draw.cpp"
#include "imgui_demo.cpp"
#endif//QUICK_APP_IMGUI

namespace QuickApp {

namespace Input {
  static int globalMouseX = 0;
  static int globalMouseY = 0;
  static int localMouseX = 0;
  static int localMouseY = 0;
  static int deltaMouseX = 0;
  static int deltaMouseY = 0;
};


namespace Internal {
static bool isRunning = false;
static SDL_Window *window;
static int screenWidth;
static int screenHeight;
static int lastMouseX;
static int lastMouseY;
#ifdef QUICKAPP_IMGUI
static double time = 0.0f;
static bool mousePressed[3] = {};
static float mouseWheel = 0.0f;
static GLuint fontTexture = 0;
#endif//QUICKAPP_IMGUI
};

#ifdef QUICKAPP_IMGUI

// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
// If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_ImplSdl_RenderDrawLists(ImDrawData* draw_data) {
    // Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
    int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
    if (fb_width == 0 || fb_height == 0)
        return;
    draw_data->ScaleClipRects(io.DisplayFramebufferScale);

    // We are using the OpenGL fixed pipeline to make the example code simpler to read!
    // Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, vertex/texcoord/color pointers.
    GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
    GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box); 
    glPushAttrib(GL_ENABLE_BIT | GL_COLOR_BUFFER_BIT | GL_TRANSFORM_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context

    // Setup viewport, orthographic projection matrix
    glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0.0f, io.DisplaySize.x, io.DisplaySize.y, 0.0f, -1.0f, +1.0f);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Render command lists
    #define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
    for (int n = 0; n < draw_data->CmdListsCount; n++)
    {
        const ImDrawList* cmd_list = draw_data->CmdLists[n];
        const ImDrawVert* vtx_buffer = cmd_list->VtxBuffer.Data;
        const ImDrawIdx* idx_buffer = cmd_list->IdxBuffer.Data;
        glVertexPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, pos)));
        glTexCoordPointer(2, GL_FLOAT, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, uv)));
        glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(ImDrawVert), (const GLvoid*)((const char*)vtx_buffer + OFFSETOF(ImDrawVert, col)));

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
        {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback)
            {
                pcmd->UserCallback(cmd_list, pcmd);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
                glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
                glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer);
            }
            idx_buffer += pcmd->ElemCount;
        }
    }
    #undef OFFSETOF

    // Restore modified state
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glBindTexture(GL_TEXTURE_2D, (GLuint)last_texture);
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
    glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
}

static const char* ImGui_ImplSdl_GetClipboardText(void*)
{
    return SDL_GetClipboardText();
}

static void ImGui_ImplSdl_SetClipboardText(void*, const char* text)
{
    SDL_SetClipboardText(text);
}

bool ImGui_ImplSdl_ProcessEvent(SDL_Event* event)
{
    ImGuiIO& io = ImGui::GetIO();
    switch (event->type)
    {
    case SDL_MOUSEWHEEL:
        {
            if (event->wheel.y > 0)
                Internal::mouseWheel = 1;
            if (event->wheel.y < 0)
              Internal::mouseWheel = -1;
            return true;
        }
    case SDL_MOUSEBUTTONDOWN:
        {
            if (event->button.button == SDL_BUTTON_LEFT) Internal::mousePressed[0] = true;
            if (event->button.button == SDL_BUTTON_RIGHT) Internal::mousePressed[1] = true;
            if (event->button.button == SDL_BUTTON_MIDDLE) Internal::mousePressed[2] = true;
            return true;
        }
    case SDL_TEXTINPUT:
        {
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
    case SDL_KEYDOWN:
    case SDL_KEYUP:
        {
            int key = event->key.keysym.sym & ~SDLK_SCANCODE_MASK;
            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
            return true;
        }
    }
    return false;
}

bool ImGui_ImplSdl_CreateDeviceObjects()
{
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;
#ifdef QUICKAPP_IMGUI_FONT_FILE
    io.Fonts->AddFontFromFileTTF(QUICKAPP_IMGUI_FONT_FILE, QUICKAPP_IMGUI_FONT_SIZE);
#endif//QUICKAPP_IMGUI_FONT_FILE
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);

    // Upload texture to graphics system
    GLint last_texture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
    glGenTextures(1, &Internal::fontTexture);
    glBindTexture(GL_TEXTURE_2D, Internal::fontTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = (void *)(intptr_t)Internal::fontTexture;

    // Restore state
    glBindTexture(GL_TEXTURE_2D, last_texture);

    return true;
}

void    ImGui_ImplSdl_InvalidateDeviceObjects()
{
    if (Internal::fontTexture)
    {
        glDeleteTextures(1, &Internal::fontTexture);
        ImGui::GetIO().Fonts->TexID = 0;
        Internal::fontTexture = 0;
    }
}

bool    ImGui_ImplSdl_Init(SDL_Window* window)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeyMap[ImGuiKey_Tab] = SDLK_TAB;                     // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
    io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
    io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
    io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
    io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
    io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
    io.KeyMap[ImGuiKey_Delete] = SDLK_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = SDLK_BACKSPACE;
    io.KeyMap[ImGuiKey_Enter] = SDLK_RETURN;
    io.KeyMap[ImGuiKey_Escape] = SDLK_ESCAPE;
    io.KeyMap[ImGuiKey_A] = SDLK_a;
    io.KeyMap[ImGuiKey_C] = SDLK_c;
    io.KeyMap[ImGuiKey_V] = SDLK_v;
    io.KeyMap[ImGuiKey_X] = SDLK_x;
    io.KeyMap[ImGuiKey_Y] = SDLK_y;
    io.KeyMap[ImGuiKey_Z] = SDLK_z;

    io.RenderDrawListsFn = ImGui_ImplSdl_RenderDrawLists;   // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
    io.SetClipboardTextFn = ImGui_ImplSdl_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplSdl_GetClipboardText;
    io.ClipboardUserData = NULL;

#ifdef _WIN32
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    SDL_GetWindowWMInfo(window, &wmInfo);
    io.ImeWindowHandle = wmInfo.info.win.window;
#else
    (void)window;
#endif

    return true;
}

void ImGui_ImplSdl_Shutdown()
{
    ImGui_ImplSdl_InvalidateDeviceObjects();
    ImGui::Shutdown();
}

void ImGui_ImplSdl_NewFrame(SDL_Window *window)
{
    if (!Internal::fontTexture)
        ImGui_ImplSdl_CreateDeviceObjects();

    ImGuiIO& io = ImGui::GetIO();

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

    // Setup time step
    Uint32	time = SDL_GetTicks();
    double current_time = time / 1000.0;
    io.DeltaTime = Internal::time > 0.0 ? (float)(current_time - Internal::time) : (float)(1.0f/60.0f);
    Internal::time = current_time;

    // Setup inputs
    // (we already got mouse wheel, keyboard keys & characters from SDL_PollEvent())
    int mx, my;
    Uint32 mouseMask = SDL_GetMouseState(&mx, &my);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)
        io.MousePos = ImVec2((float)mx, (float)my);   // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
    else
        io.MousePos = ImVec2(-1,-1);

    io.MouseDown[0] = Internal::mousePressed[0] || (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;		// If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
    io.MouseDown[1] = Internal::mousePressed[1] || (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
    io.MouseDown[2] = Internal::mousePressed[2] || (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
    Internal::mousePressed[0] = Internal::mousePressed[1] = Internal::mousePressed[2] = false;

    io.MouseWheel = Internal::mouseWheel;
    Internal::mouseWheel = 0.0f;

    // Hide OS mouse cursor if ImGui is drawing it
    SDL_ShowCursor(io.MouseDrawCursor ? 0 : 1);

    // Start the frame
    ImGui::NewFrame();
}

#endif//QUICKAPP_IMGUI

int Start(const char *title, int width, int height) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) return 0;
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  Internal::window = SDL_CreateWindow(title, 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS);
  SDL_GLContext glcontext = SDL_GL_CreateContext(Internal::window);
  Internal::isRunning = true;
  Internal::screenWidth = width;
  Internal::screenHeight = height;
  #ifdef QUICKAPP_IMGUI
  ImGui_ImplSdl_Init(Internal::window);
  #endif//QUICKAPP_IMGUI
  return 1;
}

void Loop(std::function<void()> loopProc, std::function<void(SDL_Event *)> eventProc) {
  while (Internal::isRunning){
    SDL_Event event;

    Input::deltaMouseX = 0;
    Input::deltaMouseY = 0;

    while (SDL_PollEvent(&event)) {
      eventProc(&event);
      #ifdef QUICKAPP_IMGUI
      ImGui_ImplSdl_ProcessEvent(&event);
      #endif//QUICKAPP_IMGUI
      if(event.type == SDL_QUIT) 
        Internal::isRunning = false;
    }

    { //Update mouse input state
      int mouseX, mouseY;
      SDL_GetGlobalMouseState(&mouseX, &mouseY);
      Input::deltaMouseX = mouseX - Input::globalMouseX;
      Input::deltaMouseY = mouseY - Input::globalMouseY;
      Input::globalMouseX = mouseX;
      Input::globalMouseY = mouseY;
      SDL_GetMouseState(&mouseX, &mouseY);
      Input::localMouseX = mouseX;
      Input::localMouseY = mouseY;
    }

    #ifdef QUICKAPP_IMGUI
    ImGui_ImplSdl_NewFrame(Internal::window);
    #endif//QUICKAPP_IMGUI
    loopProc();
    glViewport(0, 0, Internal::screenWidth, Internal::screenHeight);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    #ifdef QUICKAPP_IMGUI
    ImGui::Render();
    #endif//QUICKAPP_IMGUI
    SDL_GL_SwapWindow(Internal::window);
  }
}

}; //namespace Quickapp

#endif//QUICKAPP_IMPLEMENTATION