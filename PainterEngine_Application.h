#ifndef PAINTERENGINE_APPLICATION_H
#define PAINTERENGINE_APPLICATION_H

#ifdef __cplusplus
extern "C"{
#endif

#include "PainterEngine_Startup.h"

#ifdef __cplusplus
}
#endif
typedef struct
{
    PX_Runtime runtime;
} PX_Application;

extern PX_Application App;

px_bool PX_ApplicationInitialize(PX_Application *App, px_int screen_Width, px_int screen_Height);

px_void PX_ApplicationUpdate(PX_Application *App, px_dword elpased);

px_void PX_ApplicationRender(PX_Application *App, px_dword elpased);

px_void PX_ApplicationPostEvent(PX_Application *App, PX_Object_Event e);

/////////////////////// min max ////////////////////////

#define min(a, b) ((a)<(b)?(a):(b))
#define max(a, b) ((a)>(b)?(a):(b))

////////////////// init functions ///////////////////////

px_void larger();

px_void smaller();

px_int posRToW(px_int pos, px_bool x);

px_int posWToR(px_int pos, px_bool x);

px_int ceilFloor(int number);

px_void deleteSelection();

px_void draw(px_int y, px_int x, px_bool color, px_bool whichMap);

px_void caculate();

px_int APPExplorerGetPathFolderCount(const px_char *path, const char *filter);

px_int APPExplorerGetPathFileCount(const px_char *path, const char *filter);

px_int APPExplorerGetPathFolderName(const char path[], int count, char FileName[][260], const char *filter);

px_int APPExplorerGetPathFileName(const char path[], int count, char FileName[][260], const char *filter);

#endif

////////////////// init keys ///////////////////////

#ifdef __WINDOWS_
#define LEFT 39
#define RIGHT 37
#define SPACE 32
#define ESC 27
#define I 73
#define V 86
#define G 103
#define BACKSPACE 127
#endif
#ifdef __APPLE__
#define LEFT 102
#define RIGHT 100
#define SPACE 32
#define ESC 27
#define I 105
#define V 118
#define G 103
#define BACKSPACE 127
#define MINUS 45
#define EQUAL 61
#define H 104
#endif