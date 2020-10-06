#include "WindowManager.hxx"
#include "LOG.hxx"
#include "Exception.hxx"
#include "basics/Settings.hxx"
#include "Filesystem.hxx"
#include "../renderer/SDLRenderer.hxx"
#include "../view/Window.hxx"
#include <SDL_image.h>

WindowManager::WindowManager()
{
  m_numOfDisplays = SDL_GetNumVideoDisplays();
  // This is broken. It leaked the display modes and also looped over an extra display
  //initializeScreenResolutions();
}

WindowManager::~WindowManager()
{
  LOG(LOG_DEBUG) << "Destroying WindowManager";
}

void WindowManager::toggleFullScreen() const
{
  Settings::instance().fullScreen = !Settings::instance().fullScreen;

  if (Settings::instance().fullScreen)
  {
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
  }
  else
  {
    SDL_SetWindowFullscreen(m_window, 0);
  }
}

void WindowManager::setRealWindow(Window & window)
{
  m_window = window.m_Window;
  m_renderer = dynamic_cast<SDLRenderer&>(*window.m_Renderer).m_Renderer;
}

void WindowManager::setFullScreenMode(FULLSCREEN_MODE mode) const
{
  Settings::instance().fullScreenMode = static_cast<int>(mode);

  // reset the actual resolution back to the desired resolution
  Settings::instance().currentScreenHeight = Settings::instance().screenHeight;
  Settings::instance().currentScreenWidth = Settings::instance().screenWidth;
  switch (mode)
  {
  case FULLSCREEN_MODE::WINDOWED:
    SDL_SetWindowFullscreen(m_window, 0);
    break;
  case FULLSCREEN_MODE::FULLSCREEN:
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
    break;
  case FULLSCREEN_MODE::BORDERLESS:
    SDL_DisplayMode desktopScreenMode;
    if (SDL_GetDesktopDisplayMode(0, &desktopScreenMode) != 0)
    {
      LOG(LOG_INFO) << "SDL_GetDesktopDisplayMode failed: " << SDL_GetError();
    }
    // set the actual resolution to the desktop resolution for Borderless
    Settings::instance().currentScreenHeight = desktopScreenMode.h;
    Settings::instance().currentScreenWidth = desktopScreenMode.w;

    // As a workaround, need to switch back into windowed mode before changing the display mode, then back to full screen mode.
    // Minimize / Restore is another workaround to get the change from fullscreen to Borderless working
    SDL_SetWindowFullscreen(m_window, 0);
    SDL_MinimizeWindow(m_window);
    SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_RestoreWindow(m_window);

    break;
  default:
    // do nothing or log message on the off chance this is reached
    break;
  }
}

void WindowManager::setWindowTitle(const std::string &title)
{
  m_title = title;
  SDL_SetWindowTitle(m_window, m_title.c_str());
}

void WindowManager::initializeScreenResolutions()
{
  if (m_activeDisplay > m_numOfDisplays)
    throw UIError(TRACE_INFO "There is no display with number " + std::to_string(m_activeDisplay) + ". Resetting to display 0");

  // get the number of different screen modes
  for (int modeIndex = 0; modeIndex < SDL_GetNumDisplayModes(m_activeDisplay); modeIndex++)
  {
    SDL_DisplayMode *mode = new SDL_DisplayMode{SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, nullptr};

    if (SDL_GetDisplayMode(m_activeDisplay, modeIndex, mode) == 0)
    {
      m_resolutions.push_back(mode);
    }
  }
}

void WindowManager::setScreenResolution(int mode)
{
  // check if the desired mode exists first
  if (mode >= 0 && mode < static_cast<int>(m_resolutions.size()) && m_resolutions[mode])
  {
    Settings::instance().screenWidth = m_resolutions[mode]->w;
    Settings::instance().screenHeight = m_resolutions[mode]->h;

    // update the actual resolution
    Settings::instance().currentScreenWidth = Settings::instance().screenWidth;
    Settings::instance().currentScreenHeight = Settings::instance().screenHeight;

    switch (static_cast<FULLSCREEN_MODE>(Settings::instance().fullScreenMode))
    {
    case FULLSCREEN_MODE::FULLSCREEN:
      SDL_SetWindowDisplayMode(m_window, m_resolutions[mode]);
      // workaround. After setting Display Resolution in fullscreen, it won't work until disabling / enabling Fullscreen again.
      SDL_SetWindowFullscreen(m_window, 0);
      SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
      break;
    case FULLSCREEN_MODE::WINDOWED:
      SDL_SetWindowSize(m_window, m_resolutions[mode]->w, m_resolutions[mode]->h);
      SDL_SetWindowPosition(m_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
      break;
    default: //AKA FULLSCREEN_MODE::BORDERLESS
      // do nothing for Borderless fullscreen, it's always the screenSize
      break;
    }
  }
  else
  {
    throw UIError(TRACE_INFO "Cannot set screen mode " + std::to_string(mode));
  }
}
