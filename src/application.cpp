#include <stdexcept>
#include <string>
#include <SDL2/SDL.h>

#include "application.h"

const int default_win_width = 800;
const int default_win_height = 600;
const std::string default_win_title = "Nonny";

Application::Application() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) SDL_error("SDL_Init");

  window = SDL_CreateWindow(default_win_title.c_str(),
                            SDL_WINDOWPOS_CENTERED,
                            SDL_WINDOWPOS_CENTERED,
                            default_win_width, default_win_height,
                            SDL_WINDOW_RESIZABLE);

  if (!window) SDL_error("SDL_CreateWindow");
}

Application::~Application() {
  SDL_cleanup();
}

void Application::SDL_error(const std::string& function) {
  std::string err_msg = function;
  err_msg += ": ";
  err_msg += SDL_GetError();

  SDL_cleanup();
  throw std::runtime_error(err_msg);
}

void Application::SDL_cleanup() {
  if (window) SDL_DestroyWindow(window);
  SDL_Quit();
}

void Application::run() {
}
