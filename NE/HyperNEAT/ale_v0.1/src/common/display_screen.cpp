#include "display_screen.h"

DisplayScreen::DisplayScreen(bool use_bass) {
  /* Initialise SDL Video */
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    printf("Could not initialize SDL: %s\n", SDL_GetError());
    exit(1);
  }

  screen_width = 337;
  if (use_bass)
    screen_height = 750;
  else
    screen_height = 420;

	
  /* Open a 640 x 480 screen */
  screen = SDL_SetVideoMode(screen_width,screen_height, 0, SDL_HWPALETTE|SDL_DOUBLEBUF);
	
  if (screen == NULL) {
    printf("Couldn't set screen mode to 640 x 480: %s\n", SDL_GetError());
    exit(1);
  }

  //atexit(SDL_Quit);
  //atexit(exit);

  /* Set the screen title */
  //SDL_WM_SetCaption(romName, NULL);
}

DisplayScreen::~DisplayScreen() {
  /* Free the image */
  // if (image != NULL) {
  //   SDL_FreeSurface(image);
  // }
	
  /* Shut down SDL */
  SDL_Quit();
}

void DisplayScreen::display_png(const string& filename) {
  poll(); // Check for quit event
  image = IMG_Load(filename.c_str());
  if ( !image ) {
    printf ( "IMG_Load: %s\n", IMG_GetError () );
  } 


  // Draws the image on the screen:
  SDL_Rect rcDest = { 0, 0, 2*image->w, 2*image->h };
  SDL_Surface *image2 = zoomSurface(image, 2.0, 2.0, 0);
  SDL_BlitSurface ( image2, NULL, screen, &rcDest );
  // something like SDL_UpdateRect(surface, x_pos, y_pos, image->w, image->h); is missing here

  SDL_Flip(screen);

  SDL_FreeSurface(image);
  SDL_FreeSurface(image2);
  SDL_FreeSurface(screen);
  //SDL_Delay(16);
}

void DisplayScreen::display_bass_png(const string& filename) {
  poll(); // Check for quit event
  image = IMG_Load(filename.c_str());
  if ( !image ) {
    printf ( "IMG_Load: %s\n", IMG_GetError () );
  } 

  // Draws the image on the screen:
  int scale = 3;
  int x = (screen_width - scale*image->w)/2;
  SDL_Rect rcDest = { x, 420, scale*image->w, scale*image->h };
  SDL_Surface *image2 = zoomSurface(image, scale, scale, 0);
  SDL_BlitSurface ( image2, NULL, screen, &rcDest );
  // something like SDL_UpdateRect(surface, x_pos, y_pos, image->w, image->h); is missing here

  SDL_Flip(screen);

  SDL_FreeSurface(image);
  SDL_FreeSurface(image2);
  SDL_FreeSurface(screen);
  //SDL_Delay(16);
}

void DisplayScreen::poll() {
  SDL_Event event;
  while(SDL_PollEvent(&event)) {
    switch (event.type)
      {
      case SDL_QUIT:
        exit(0);
        break;
      }
  }
};



// SDL_Surface *loadImage(char *name)
// {
//   /* Load the image using SDL Image */
//   SDL_Surface *temp = IMG_Load(name);
//   SDL_Surface *image;
	
//   if (temp == NULL) {
//     printf("Failed to load image %s\n", name);
//     return NULL;
//   }
	
//   /* Make the background transparent */
//   SDL_SetColorKey(temp, (SDL_SRCCOLORKEY|SDL_RLEACCEL), SDL_MapRGB(temp->format, 0, 0, 0));
	
//   /* Convert the image to the screen's native format */
//   image = SDL_DisplayFormat(temp);
	
//   SDL_FreeSurface(temp);
	
//   if (image == NULL) {
//     printf("Failed to convert image %s to native format\n", name);
//     return NULL;
//   }
	
//   /* Return the processed image */
//   return image;
// }
