#ifndef DISPLAY_SCREEN_H
#define DISPLAY_SCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_rotozoom.h"
#include "SDL/SDL_gfxPrimitives.h"
#include "common_constants.h"
#include "export_screen.h"

class SDLEventHandler {
 public:
  virtual void handleSDLEvent(const SDL_Event& event) = 0;
};

class DisplayScreen {
 public:
  DisplayScreen(bool use_bass, ExportScreen* export_screen);
  virtual ~DisplayScreen();

  void display_screen(const IntMatrix& screen_matrix, int image_widht, int image_height);
  void display_png(const string& filename);
  void display_bass_png(const string& filename);
  void poll();
  void registerEventHandler(SDLEventHandler* handler) { handlers.push_back(handler); };

  int screen_height;
  int screen_width;

  bool paused;
  bool poll_for_events;
  
  SDL_Surface *screen, *image;
  ExportScreen* export_screen;

  std::vector<SDLEventHandler*> handlers;
};

#endif
