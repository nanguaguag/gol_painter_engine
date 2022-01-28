#include "PainterEngine_Application.h"

PX_Application App;

/////////////////// PX ////////////////////////////

PX_FontModule fm;
PX_Object *root;
PX_Object *messagebox;
//PX_Object *explorer;

/////////////////// init world /////////////////////////

px_bool worldMap[2][2187][2187]; // 地图 (3^7 * 3^7)
px_bool currentMap = 0;          // 当前使用的worldMap
px_bool map3x3[3][3];
px_bool map9x9[9][9];
px_bool map27x27[27][27];
px_bool map81x81[81][81];
px_bool map243x243[243][243];
px_bool map729x729[729][729];

/////////////////// init values /////////////////////////

px_int selection[4];   // up, down, left, right
px_int around[2][8] = {{1,  1, 1, 0,  0, -1, -1, -1},
                       {-1, 0, 1, -1, 1, -1, 0,  1}};
px_int screenPosX = 1280, screenPosY = 800, cellSize = 32; // 视线区域
px_int lastScreenPosX, lastScreenPosY;

px_int kd;   // 键盘事件
struct Mouse // 鼠标事件
{
    px_int x;
    px_int y;
    px_int downPosX;
    px_int downPosY;
    px_bool down;
} mouse;

/////////////////// 状态 //////////////////////////

px_bool pause = 1;        // 是否暂停（空格键继续）
px_bool drawingPause = 0; // 是否暂停（空格键继续）
px_bool selecting = 0;    // 是否选择中
px_bool showGrids = 0;    // 是否画网格
px_int status = 0;        // normal = 0, insert = 1, visual = 2;
px_char strStatus[3][7] = {"NORMAL", "INSERT", "VISUAL"};
px_int pickedColor = 1;
px_int speed = 1, countSpeed = 0; // 运算速度

////////////////////// colors /////////////////////////

px_color red, green, blue, yellow, white, black, bgGrey, grey1, grey2;

////////////////////// end /////////////////////////

px_bool PX_ApplicationInitialize(PX_Application *pApp, px_int screen_width, px_int screen_height)
{
    PX_ApplicationInitializeDefault(&pApp->runtime, screen_width, screen_height);
    if (!PX_FontModuleInitialize(&pApp->runtime.mp_resources, &fm)) return PX_FALSE;
    if (!PX_LoadFontModuleFromFile(&fm, "../assets/utf8_32.pxf")) return PX_FALSE;
    root = PX_ObjectCreate(&pApp->runtime.mp_ui, PX_NULL, 0, 0, 0, 0, 0, 0);
    messagebox = PX_Object_MessageBoxCreate(&pApp->runtime.mp_ui, root, &fm);
/*
    explorer = PX_Object_ExplorerCreate(&pApp->runtime.mp_ui, PX_NULL, 0, 0,
                                        pApp->runtime.surface_width,
                                        pApp->runtime.surface_height, &fm,
                                        APPExplorerGetPathFolderCount,
                                        APPExplorerGetPathFileCount,
                                        APPExplorerGetPathFolderName,
                                        APPExplorerGetPathFileName, "");
    PX_Object_ExplorerOpen(explorer);
*/
//                                        APPExplorerGetPathFolderName,
//                                        APPExplorerGetPathFileName, "");
//    PX_Object_ExplorerOpen(explorer);

    red = PX_COLOR(100, 255, 0, 0);
    green = PX_COLOR(50, 0, 255, 0);
    blue = PX_COLOR(100, 0, 0, 255);
    yellow = PX_COLOR(255, 255, 255, 0);
    white = PX_COLOR(255, 255, 255, 255);
    black = PX_COLOR(255, 0, 0, 0);
    bgGrey = PX_COLOR(255, 210, 210, 210);
    grey1 = PX_COLOR(200, 48, 48, 48);
    grey2 = PX_COLOR(100, 64, 64, 64);
    return PX_TRUE;
}

px_void PX_ApplicationUpdate(PX_Application *pApp, px_dword elpased)
{
    //    printf("%d\n", elpased);
    if (!pause && !drawingPause)
    {
        if (speed > 0)
        {
            for (int i = 0; i < speed; ++i)
                caculate();
        } else if (countSpeed + speed > 0)
        {
            caculate();
            countSpeed = 0;
        } else countSpeed++;
    }
}

px_void PX_ApplicationRender(PX_Application *pApp, px_dword elpased)
{
    px_surface *pRenderSurface = &pApp->runtime.RenderSurface;
    if (cellSize < 4 && showGrids) PX_RuntimeRenderClear(&pApp->runtime, bgGrey);
    else PX_RuntimeRenderClear(&pApp->runtime, white);

    // 画ceils细胞方块
    if (screenPosX < 0) screenPosX = 0;
    if (screenPosX > 2185 - pApp->runtime.surface_width / cellSize)
        screenPosX = 2185 - pApp->runtime.surface_width / cellSize;
    if (screenPosY < 0) screenPosY = 0;
    if (screenPosY > 2185 - pApp->runtime.surface_height / cellSize)
        screenPosY = 2185 - pApp->runtime.surface_height / cellSize;
    for (int j = 0; j <= pApp->runtime.surface_width / cellSize; ++j)
        for (int i = 0; i <= pApp->runtime.surface_height / cellSize; ++i)
            if (worldMap[currentMap][screenPosY + i][screenPosX + j])
                PX_GeoDrawRect(pRenderSurface, j * cellSize, i * cellSize,
                               (j + 1) * cellSize, (i + 1) * cellSize, black);

//    for (int j = 0; j <= pApp->runtime.surface_width / cellSize; ++j)
//        for (int i = 0; i <= pApp->runtime.surface_height / cellSize; ++i)
//            if (map729x729[(screenPosY + i) / 3][(screenPosX + j) / 3])
//                PX_GeoDrawRect(pRenderSurface, j * cellSize, i * cellSize,
//                               (j + 1) * cellSize, (i + 1) * cellSize, red);

//    for (int j = 0; j <= pApp->runtime.surface_width / cellSize; ++j)
//        for (int i = 0; i <= pApp->runtime.surface_height / cellSize; ++i)
//            if (map243x243[(screenPosY + i) / 9][(screenPosX + j) / 9])
//                PX_GeoDrawRect(pRenderSurface, j * cellSize, i * cellSize,
//                               (j + 1) * cellSize, (i + 1) * cellSize, green);

    if (status == 1)
    { // 画鼠标hover
        PX_GeoDrawRect(pRenderSurface, ceilFloor(mouse.x), ceilFloor(mouse.y),
                       (mouse.x / cellSize + 1) * cellSize,
                       ceilFloor(mouse.y) + cellSize, green);
    } else if (status == 2)
    {
        px_int X = ceilFloor(mouse.x + cellSize / 2), Y = ceilFloor(mouse.y + cellSize / 2);
        PX_GeoDrawLine(pRenderSurface, X - 10, Y, X + 10, Y, 2, mouse.down ? red : blue);// 画鼠标十字(横)
        PX_GeoDrawLine(pRenderSurface, X, Y - 10, X, Y + 10, 2, mouse.down ? red : blue); // 画鼠标十字(竖)
        PX_GeoDrawSolidCircle(pRenderSurface, X, Y, 4, mouse.down ? red : blue); // 中心点
    }
    if (selecting) // 画选择框
        PX_GeoDrawRect(pRenderSurface, posWToR(selection[2], 1),
                       posWToR(selection[0], 0), posWToR(selection[3], 1),
                       posWToR(selection[1], 0), green);
    if (showGrids) // 画网格
    {
        for (int j = 0; j <= pApp->runtime.surface_width / cellSize; ++j)
        {
            if ((screenPosX + j) % 10 == 0)
            {
                PX_GeoDrawLine(pRenderSurface, j * cellSize, 0, j * cellSize,
                               pApp->runtime.surface_height, 1, grey1);
                continue; // 大网格（10 * 10）
            }
            if (cellSize > 2) // 小网格（1 * 1）
                PX_GeoDrawLine(pRenderSurface, j * cellSize, 0, j * cellSize,
                               pApp->runtime.surface_height, 1, grey2);
        }
        for (int i = 0; i <= pApp->runtime.surface_height / cellSize; ++i)
        {
            if ((screenPosY + i) % 10 == 0)
            {
                PX_GeoDrawLine(pRenderSurface, 0, i * cellSize,
                               pApp->runtime.surface_width, i * cellSize, 1, grey1);
                continue; // 大网格（10 * 10）
            }
            if (cellSize > 2) // 小网格（1 * 1）
                PX_GeoDrawLine(pRenderSurface, 0, i * cellSize,
                               pApp->runtime.surface_width, i * cellSize, 1, grey2);
        }
    }
    // 画world范围
    PX_GeoDrawBorder(pRenderSurface, posWToR(0, 1), posWToR(0, 0),
                     posWToR(2186, 1), posWToR(2186, 0), cellSize, red);
    char content[64];
    PX_sprintf1(content, sizeof(content), "< %1 >", PX_STRINGFORMAT_STRING(strStatus[status]));
    PX_FontModuleDrawText(pRenderSurface, &fm, 0, pApp->runtime.surface_height, PX_ALIGN_LEFTBOTTOM, content, black);
    PX_ObjectRender(pRenderSurface, root, elpased);
}

px_void PX_ApplicationPostEvent(PX_Application *pApp, PX_Object_Event e)
{
    PX_ApplicationEventDefault(&pApp->runtime, e);
    PX_ObjectPostEvent(root, e);
    if (e.Event == PX_OBJECT_EVENT_KEYDOWN)
    {
        kd = PX_Object_Event_GetKeyDown(e);
        if (kd == LEFT) larger(); // LEFT : 102 on mac || 39 on windows || 1103 on web
        else if (kd == RIGHT) smaller(); // RIGHT : 100 on mac || 37 on windows || 1104 on web
        else if (kd == SPACE) pause = !pause; // SPACE: 32 on mac and windows
        else if (kd == ESC) status = 0; // ESC : 27 on mac and windows
        else if (kd == I) status = 1; // i : 105 on mac || 73 on windows
        else if (kd == V) status = 2; // v : 118 on mac || 86 on windows
        else if (kd == G) showGrids = !showGrids; // g : 103 on mac ||  on windows
        else if (kd == BACKSPACE) deleteSelection(); // BACKSPACE : 127 on mac || 8 on windows || 8 on web
        else if (kd == MINUS) speed--; // accelerate | speed  up
        else if (kd == EQUAL) speed++; // decelerate | slow down
        else if (kd == Hh) // help message
        {
            PX_Object_MessageBoxAlertOk(messagebox, "HELP: <I> enter Insert mode | <V> enter Visual mode | <ESC> enter Normal mode\n"
                                                    "<-> slow down | <+> speed up | <G> show grids",
                                        PX_NULL, PX_NULL);
            //3.press <SPACE> to start or stop\n 4.press <G> to show grids\n
        } else printf("keydown not found:%d\n", kd);
    }
    if (e.Event == PX_OBJECT_EVENT_CURSORMOVE)
    {
        mouse.x = PX_Object_Event_GetCursorX(e);
        mouse.y = PX_Object_Event_GetCursorY(e);
    }
    if (e.Event == PX_OBJECT_EVENT_CURSORDRAG)
    {
        drawingPause = 1;
        px_int lastPosX = mouse.x, lastPosY = mouse.y; // 备份mouse坐标 x, y
        mouse.x = PX_Object_Event_GetCursorX(e);       // 更新mouse坐标 x
        mouse.y = PX_Object_Event_GetCursorY(e);       // 更新mouse坐标 y
        px_int X = posRToW(mouse.x, 1), lpX = posRToW(lastPosX, 1); // last position X
        px_int Y = posRToW(mouse.y, 0), lpY = posRToW(lastPosY, 0); // last position Y
        if (status == 0)
        {
            screenPosX = (mouse.downPosX - mouse.x) / cellSize + lastScreenPosX;
            screenPosY = (mouse.downPosY - mouse.y) / cellSize + lastScreenPosY;
        } else if (status == 1)
        {
            int dx = X - lpX, dy = Y - lpY;
            if (PX_ABS(dx) > PX_ABS(dy))
            {
                for (int j = min(X, lpX); j <= max(X, lpX); ++j)
                    draw((int) (lpY + (float) (j - lpX) * (float) dy / (float) dx), j, pickedColor, currentMap);
            } else if (PX_ABS(dx) < PX_ABS(dy))
            {
                for (int i = min(Y, lpY); i <= max(Y, lpY); ++i)
                    draw(i, (int) (lpX + (float) (i - lpY) * (float) dx / (float) dy), pickedColor, currentMap);
            } else
            {
                if (dx > 0)
                {
                    for (int i = 1; i <= dy; ++i) draw(lpY + i, lpX + i, pickedColor, currentMap);
                    for (int i = dy; i <= -1; ++i) draw(lpY + i, lpX - i, pickedColor, currentMap);
                } else if (dx < 0)
                {
                    for (int i = 1; i <= dy; ++i) draw(lpY + i, lpX - i, pickedColor, currentMap);
                    for (int i = dy; i <= -1; ++i) draw(lpY + i, lpX + i, pickedColor, currentMap);
                }
            }
        } else if (status == 2)
        {
            selection[0] = min(posRToW(mouse.downPosY + cellSize / 2, 0), posRToW(mouse.y + cellSize / 2, 0)); // up
            selection[1] = max(posRToW(mouse.downPosY + cellSize / 2, 0), posRToW(mouse.y + cellSize / 2, 0)); // down
            selection[2] = min(posRToW(mouse.downPosX + cellSize / 2, 1), posRToW(mouse.x + cellSize / 2, 1)); // left
            selection[3] = max(posRToW(mouse.downPosX + cellSize / 2, 1), posRToW(mouse.x + cellSize / 2, 1)); // right
            selecting = 1;
        }
    } else if (e.Event == PX_OBJECT_EVENT_CURSORDOWN) // 不是drag就只能是click了
    {
        mouse.down = 1;
        mouse.downPosX = mouse.x;
        mouse.downPosY = mouse.y;
        px_int X = mouse.x / cellSize + screenPosX, Y = mouse.y / cellSize + screenPosY;
        switch (status)
        {
            case 0: // NOMAL 模式
            {
                lastScreenPosX = screenPosX;
                lastScreenPosY = screenPosY;
                break;
            }
            case 1: // 点击屏幕 + 选中颜色
            {
                pickedColor = !worldMap[currentMap][Y][X];
                draw(Y, X, pickedColor, currentMap);
                break;
            }
            case 2: // VISUAL
            {
                if (selecting) selecting = 0;
                break;
            }
            default:
            {
                break;
            }
        }
    } else if (e.Event == PX_OBJECT_EVENT_CURSORUP) // 左键抬起
    {
        drawingPause = 0;
        mouse.down = 0;
    } else if (e.Event == PX_OBJECT_EVENT_CURSORWHEEL) // useless on mac
    {
        if (PX_Object_Event_GetCursorIndex(e) > 0) larger(); // 鼠标滚动
        else smaller();
    }
    if (e.Event == PX_OBJECT_EVENT_DRAGFILE)
        printf("aaaaaaaaaaaa");
}

px_void larger()
{
    if (cellSize <= 32)
    {
        screenPosX += PX_ABS((mouse.x / cellSize) - (mouse.x / (cellSize * 2)));
        screenPosY += PX_ABS((mouse.y / cellSize) - (mouse.y / (cellSize * 2)));
        cellSize *= 2;
    }
}

px_void smaller()
{
    screenPosX -= PX_ABS((mouse.x / cellSize) - (mouse.x / (cellSize - cellSize / 2)));
    screenPosY -= PX_ABS((mouse.y / cellSize) - (mouse.y / (cellSize - cellSize / 2)));
    cellSize -= cellSize / 2;
}

px_void draw(px_int y, px_int x, px_bool color, px_bool whichMap)
{
//    printf("x:%d,y:%d\n", x, y);
    if (x < 0 || x > 2186 || y < 0 || y > 2186)
        return;
    worldMap[whichMap][y][x] = color;
    if (color)
    {
        for (int i = -1; i <= 1; i += 2)
        {
            map3x3[(y + i) / 729][x / 729] = 1;
            map9x9[(y + i) / 243][x / 243] = 1;
            map27x27[(y + i) / 81][x / 81] = 1;
            map81x81[(y + i) / 27][x / 27] = 1;
            map243x243[(y + i) / 9][x / 9] = 1;
            map729x729[(y + i) / 3][x / 3] = 1;
        }
        for (int j = -1; j <= 1; j += 2)
        {
            map3x3[y / 729][(x + j) / 729] = 1;
            map9x9[y / 243][(x + j) / 243] = 1;
            map27x27[y / 81][(x + j) / 81] = 1;
            map81x81[y / 27][(x + j) / 27] = 1;
            map243x243[y / 9][(x + j) / 9] = 1;
            map729x729[y / 3][(x + j) / 3] = 1;
        }
    }
}

px_void deleteSelection()
{
    for (int i = selection[0]; i < selection[1]; ++i)
        for (int j = selection[2]; j < selection[3]; ++j)
            draw(i, j, 0, currentMap);
}

px_void caculate()
{
    px_int countArround;
////    px_int count = 0;
    px_bool flag[10] = {0};
    for (int a = 0; a < 3; ++a) // y
    {
        for (int b = 0; b < 3; ++b) // x
        {
            if (map3x3[a][b])
            {
                // flag[1] = 1;
                for (int c = a * 3; c < a * 3 + 3; ++c)
                {
                    for (int d = b * 3; d < b * 3 + 3; ++d)
                    {
                        if (map9x9[c][d])
                        {
                            flag[2] = 1;
                            for (int e = c * 3; e < c * 3 + 3; ++e)
                            {
                                for (int f = d * 3; f < d * 3 + 3; ++f)
                                {
                                    if (map27x27[e][f])
                                    {
                                        flag[3] = 1;
                                        for (int g = e * 3; g < e * 3 + 3; ++g)
                                        {
                                            for (int h = f * 3; h < f * 3 + 3; ++h)
                                            {
                                                if (map81x81[g][h])
                                                {
                                                    flag[4] = 1;
                                                    for (int i = g * 3; i < g * 3 + 3; ++i)
                                                    {
                                                        for (int j = h * 3; j < h * 3 + 3; ++j)
                                                        {
                                                            if (map243x243[i][j])
                                                            {
                                                                flag[5] = 1;
                                                                for (int k = i * 3; k < i * 3 + 3; ++k)
                                                                {
                                                                    for (int l = j * 3; l < j * 3 + 3; ++l)
                                                                    {
                                                                        if (map729x729[k][l])
                                                                        {
                                                                            flag[6] = 1;
                                                                            for (int m = k * 3; m < k * 3 + 3; ++m)
                                                                            {
                                                                                for (int n = l * 3;
                                                                                     n < l * 3 + 3; ++n)
                                                                                {
//                                                                                    count++;
                                                                                    countArround = 0;
                                                                                    for (int o = 0; o < 8; ++o)
                                                                                    {
                                                                                        if (worldMap[currentMap][m +
                                                                                                                 around[1][o]][
                                                                                                n + around[0][o]])
                                                                                            countArround++;
                                                                                    }
                                                                                    if (countArround) flag[7] = 1;
                                                                                    else
                                                                                    {
                                                                                        for (int o = 0; o < 8; ++o)
                                                                                        {
                                                                                            if (worldMap[!currentMap][
                                                                                                    m + around[1][o]][
                                                                                                    n + around[0][o]])
                                                                                            {
                                                                                                flag[7] = 1;
                                                                                                break;
                                                                                            }
                                                                                        }
                                                                                    }
                                                                                    if (worldMap[currentMap][m][n])
                                                                                    {
                                                                                        if (countArround < 2 ||
                                                                                            countArround > 3)
                                                                                            worldMap[!currentMap][m][n] = 0;
                                                                                        else worldMap[!currentMap][m][n] = 1;
                                                                                    } else
                                                                                    {
                                                                                        if (countArround == 3)
                                                                                            draw(m, n, 1, !currentMap);
                                                                                        else worldMap[!currentMap][m][n] = 0;
                                                                                    }
                                                                                }
                                                                            }
                                                                            if (!flag[7]) map729x729[k][l] = 0;
                                                                            else flag[7] = 0;
                                                                        }
                                                                    }
                                                                }
                                                                if (!flag[6]) map243x243[i][j] = 0;
                                                                else flag[6] = 0;
                                                            }
                                                        }
                                                    }
                                                    if (!flag[5]) map81x81[g][h] = 0;
                                                    else flag[5] = 0;
                                                }
                                            }
                                        }
                                        if (!flag[4]) map27x27[e][f] = 0;
                                        else flag[4] = 0;
                                    }
                                }
                            }
                            if (!flag[3]) map9x9[c][d] = 0;
                            else flag[3] = 0;
                        }
                    }
                }
                if (!flag[2]) map3x3[a][b] = 0;
                else flag[2] = 0;
            }
        }
    }
//    printf("count:%d\n", count);
    currentMap = !currentMap;
//    pause = !pause;
}

px_int posRToW(px_int pos, px_bool x) // change real mouse position to world postion
{ return x ? pos / cellSize + screenPosX : pos / cellSize + screenPosY; }

px_int posWToR(px_int pos, px_bool x) // change real mouse position to world postion
{ return x ? (pos - screenPosX) * cellSize : (pos - screenPosY) * cellSize; }

px_int ceilFloor(int number)
{ return (number / cellSize) * cellSize; }


//px_int APPExplorerGetPathFolderCount(const px_char *path, const char *filter)
//{
//    return PX_FileGetDirectoryFileCount(path, PX_FILEENUM_TYPE_FOLDER, filter);
//}
//
//px_int APPExplorerGetPathFileCount(const px_char *path, const char *filter)
//{
//    return PX_FileGetDirectoryFileCount(path, PX_FILEENUM_TYPE_FILE, filter);
//}
//
//px_int APPExplorerGetPathFolderName(const char path[], int count, char FileName[][260], const char *filter)
//{
//    return PX_FileGetDirectoryFileName(path, count, FileName, PX_FILEENUM_TYPE_FOLDER, filter);
//}
//
//px_int APPExplorerGetPathFileName(const char path[], int count, char FileName[][260], const char *filter)
//{
//    return PX_FileGetDirectoryFileName(path, count, FileName, PX_FILEENUM_TYPE_FILE, filter);
//}
