#include "display_screen.h"


DisplayScreen::DisplayScreen(bool use_bass, ExportScreen* _export_screen):
  paused(false), poll_for_events(true), export_screen(_export_screen)
{
  /* Initialise SDL Video */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Could not initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  screen_width  = 337;
  screen_height = 420;

  if (use_bass)
    screen_height = 750;

  screen = SDL_SetVideoMode(screen_width,screen_height, 0, SDL_HWPALETTE|SDL_DOUBLEBUF|SDL_RESIZABLE);
	
  if (screen == NULL) {
    printf("Couldn't set screen mode to 640 x 480: %s\n", SDL_GetError());
    exit(1);
  }
  /* Set the screen title */
  SDL_WM_SetCaption("A.L.E. Viz", NULL);
}

DisplayScreen::~DisplayScreen() {
  /* Shut down SDL */
  SDL_Quit();
}

void DisplayScreen::display_png(const string& filename) {
  image = IMG_Load(filename.c_str());
  if ( !image ) {
    printf ( "IMG_Load: %s\n", IMG_GetError () );
  } 

  // Draws the image on the screen:
  SDL_Rect rcDest = { 0,0, (Uint16)(2*image->w), (Uint16)(2*image->h) };
  SDL_Surface *image2 = zoomSurface(image, 2.0, 2.0, 0);
  SDL_BlitSurface ( image2, NULL, screen, &rcDest );
  // something like SDL_UpdateRect(surface, x_pos, y_pos, image->w, image->h); is missing here

  SDL_Flip(screen);

  SDL_FreeSurface(image);
  SDL_FreeSurface(image2);
  SDL_FreeSurface(screen);
  if (poll_for_events)
    poll(); // Check for quit event
}

void DisplayScreen::display_screen(const IntMatrix& screen_matrix, int image_width, int image_height) {
  Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  SDL_Surface* my_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,image_width,image_height,32,rmask,gmask,bmask,amask);

  int r, g, b;
  for (int y=0; y<image_height; ++y) {
    for (int x=0; x<image_width; ++x) {
      export_screen->get_rgb_from_pallete(screen_matrix[y][x], r, g, b);
      pixelRGBA(my_surface,x,y,r,g,b,255);
    }
  }

  SDL_Surface* zoomed = zoomSurface(my_surface,screen->w/(double)image_width,screen->h/(double)image_height,0);
  SDL_BlitSurface(zoomed, NULL, screen, NULL);

  SDL_Flip(screen);
  SDL_FreeSurface(my_surface);
  SDL_FreeSurface(zoomed);
  if (poll_for_events)
    poll(); // Check for quit event
}

void DisplayScreen::display_bass_png(const string& filename) {
  image = IMG_Load(filename.c_str());
  if ( !image ) {
    printf("IMG_Load: %s\n", IMG_GetError());
  } 

  // Draws the image on the screen:
  int scale = 3;
  Uint16 x = (screen_width - scale*image->w)/2;
  SDL_Rect rcDest = { x, 420, (Uint16) (scale*image->w), (Uint16) (scale*image->h) };
  SDL_Surface *image2 = zoomSurface(image, scale, scale, 0);
  SDL_BlitSurface ( image2, NULL, screen, &rcDest );
  // something like SDL_UpdateRect(surface, x_pos, y_pos, image->w, image->h); is missing here

  SDL_Flip(screen);

  SDL_FreeSurface(image);
  SDL_FreeSurface(image2);
  SDL_FreeSurface(screen);
  //SDL_Delay(16);
  if (poll_for_events)
    poll(); // Check for quit event
}

void DisplayScreen::poll() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      exit(0);
      break;
    case SDL_VIDEORESIZE:
      screen = SDL_SetVideoMode(event.resize.w,event.resize.h, 0, SDL_HWPALETTE|SDL_DOUBLEBUF|SDL_RESIZABLE);
      break;
    // case SDL_MOUSEBUTTONDOWN:
    case SDL_KEYDOWN:
      switch(event.key.keysym.sym){
      case SDLK_SPACE:
        paused = !paused;
        while(paused) {
          poll();
          SDL_Delay(10);
        }
        break;
      default:
        break;
      }
    }
    
    // Give our event handlers a chance to deal with this event
    for (int i=0; i<handlers.size(); ++i) {
      handlers[i]->handleSDLEvent(event);
    }
  }
};
