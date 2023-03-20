#ifndef SDLKIT_H
#define SDLKIT_H

#include "SDL.h"
#define ERROR(x) error(__FILE__, __LINE__, #x)
#define VERIFY(x) do { if (!(x)) ERROR(x); } while (0)
#include <stdio.h>
#include <string.h>

static void error (const char *file, unsigned int line, const char *msg)
{
	fprintf(stderr, "[!] %s:%u  %s\n", file, line, msg);
	exit(1);
}

typedef Uint32 DWORD;
typedef Uint16 WORD;

#define DIK_SPACE SDL_SCANCODE_SPACE
#define DIK_RETURN SDL_SCANCODE_RETURN
#define DDK_WINDOW 0

#define hWndMain 0
#define hInstanceMain 0

#define Sleep(x) SDL_Delay(x)

static SDL_Keycode keys[SDL_NUM_SCANCODES];

void ddkInit();      // Will be called on startup
bool ddkCalcFrame(); // Will be called every frame, return true to continue running or false to quit
void ddkFree();      // Will be called on shutdown

class DPInput {
public:
	DPInput(int,int) {}
	~DPInput() {}
	static void Update () {}

	static bool KeyPressed(SDL_Scancode key)
	{
		bool r = keys[key];
		keys[key] = false;
		return r;
	}

};

static Uint32 *ddkscreen32;
static Uint16 *ddkscreen16;
static int ddkpitch;
static int mouse_x, mouse_y, mouse_px, mouse_py;
static bool mouse_left = false, mouse_right = false, mouse_middle = false;
static bool mouse_leftclick = false, mouse_rightclick = false, mouse_middleclick = false;

static SDL_Surface *sdlsurface = nullptr;
static SDL_Window *sdlwindow = nullptr;
static SDL_Renderer *sdlrenderer = nullptr;
static SDL_Texture *sdltexture = nullptr;

static void sdlupdate ()
{
	mouse_px = mouse_x;
	mouse_py = mouse_y;
	Uint8 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
	bool mouse_left_p = mouse_left;
	bool mouse_right_p = mouse_right;
	bool mouse_middle_p = mouse_middle;
	mouse_left = buttons & SDL_BUTTON(1);
	mouse_right = buttons & SDL_BUTTON(3);
	mouse_middle = buttons & SDL_BUTTON(2);
	mouse_leftclick = mouse_left && !mouse_left_p;
	mouse_rightclick = mouse_right && !mouse_right_p;
	mouse_middleclick = mouse_middle && !mouse_middle_p;
}

static bool ddkLock ()
{
	SDL_LockSurface(sdlsurface);
	ddkpitch = sdlsurface->pitch / (sdlsurface->format->BitsPerPixel == 32 ? 4 : 2);
	ddkscreen16 = (Uint16*)(sdlsurface->pixels);
	ddkscreen32 = (Uint32*)(sdlsurface->pixels);

  // NOTE: it appears that nobody ever checks the result of this
  // function, so idk
  return true;
}

static void ddkUnlock ()
{
	SDL_UnlockSurface(sdlsurface);
}

// https://wiki.libsdl.org/SDL2/MigrationGuide
static void ddkSetMode (int width, int height, int bpp, int refreshrate, int fullscreen, const char *title)
{
  sdlwindow = SDL_CreateWindow(title,
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               width, height,
                               fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
  VERIFY(sdlwindow);

  sdlrenderer = SDL_CreateRenderer(sdlwindow, -1, SDL_RENDERER_ACCELERATED);
  VERIFY(sdlrenderer);

  SDL_RenderSetLogicalSize(sdlrenderer, width, height);

  sdlsurface = SDL_CreateRGBSurface(0, width, height, bpp,
                                    0x00FF0000,
                                    0x0000FF00,
                                    0x000000FF,
                                    0xFF000000);

  VERIFY(sdlsurface);

  sdltexture = SDL_CreateTexture(sdlrenderer,
                                 SDL_PIXELFORMAT_ARGB8888,
                                 SDL_TEXTUREACCESS_STREAMING,
                                 width, height);

  VERIFY(sdltexture);
}

#include <gtk/gtk.h>
#include <string.h>
#include <malloc.h>

#if GTK_CHECK_VERSION(3,0,0)

static bool load_file (char *fname)
{
	char *fn;

	GtkWidget *dialog = gtk_file_chooser_dialog_new("Load a file!",
	                                                NULL,
	                                                GTK_FILE_CHOOSER_ACTION_OPEN,
	                                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                                GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
	                                                NULL
	                                                );

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		strncpy(fname, fn, 255);
		g_free(fn);

		fname[255] = 0;
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending ()) gtk_main_iteration ();

	return true;
}

static bool save_file (char *fname)
{
	char *fn;

	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save a file!",
	                                                NULL,
	                                                GTK_FILE_CHOOSER_ACTION_SAVE,
	                                                GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
	                                                GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
	                                                NULL
	                                                );

	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(dialog), TRUE);

	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		fn = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		strncpy(fname, fn, 255);
		g_free(fn);

		fname[255] = 0;
	}

	gtk_widget_destroy(dialog);

	while (gtk_events_pending ()) gtk_main_iteration ();

	return true;
}

#define FileSelectorLoad(x,file,y) load_file(file)
#define FileSelectorSave(x,file,y) save_file(file)

#else // gtk+-2.0

static char *gtkfilename;

static void selected_file (GtkWidget *button, GtkFileSelection *fs)
{
	strncpy(gtkfilename, gtk_file_selection_get_filename(fs), 255);
	gtkfilename[255] = 0;
	gtk_widget_destroy(GTK_WIDGET(fs));
	gtk_main_quit();
}

static bool select_file (char *buf)
{
	gtkfilename = buf;
	GtkWidget *dialog = gtk_file_selection_new("Let's file selection time, for enjoy!");
	g_signal_connect(G_OBJECT(dialog), "destroy", G_CALLBACK(gtk_main_quit), NULL);
	g_signal_connect(G_OBJECT(GTK_FILE_SELECTION(dialog)->ok_button), "clicked", G_CALLBACK(selected_file), G_OBJECT(dialog));
	g_signal_connect_swapped(G_OBJECT(GTK_FILE_SELECTION(dialog)->cancel_button), "clicked", G_CALLBACK(gtk_widget_destroy), G_OBJECT(dialog));
	gtk_widget_show(GTK_WIDGET(dialog));
	gtk_main();
	return *gtkfilename;
}

#define FileSelectorLoad(x,file,y) select_file(file)
#define FileSelectorSave(x,file,y) select_file(file)

#endif

static void sdlquit ()
{
	ddkFree();

  SDL_DestroyTexture(sdltexture);
  SDL_DestroyRenderer(sdlrenderer);
  SDL_DestroyWindow(sdlwindow);
	SDL_Quit();
}

static void sdlinit ()
{
	SDL_Surface *icon;
	VERIFY(!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO));

	ddkInit();

  icon = SDL_LoadBMP("/usr/share/sfxr/sfxr.bmp");
	if (!icon) {
    icon = SDL_LoadBMP("sfxr.bmp");
  }
	if (icon) {
    SDL_SetWindowIcon(sdlwindow, icon);
  }
	atexit(sdlquit);
}

static void loop (void)
{
	SDL_Event e;
	while (true)
	{
		if (SDL_PollEvent(&e)) {
			switch (e.type) {
				case SDL_QUIT:
					return;

      case SDL_KEYDOWN:
        keys[e.key.keysym.scancode] = true;
        break;

				default: break;
			}
		}

		sdlupdate();

		if (!ddkCalcFrame())
			return;

    SDL_UpdateTexture(sdltexture, NULL, sdlsurface->pixels, 640 * sizeof (Uint32));
    SDL_RenderClear(sdlrenderer);
    SDL_RenderCopy(sdlrenderer, sdltexture, NULL, NULL);
    SDL_RenderPresent(sdlrenderer);
	}
}

int main (int argc, char *argv[])
{
	gtk_init(&argc, &argv);
	sdlinit();
	loop();
	return 0;
}

#endif
