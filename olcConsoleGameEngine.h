/*
 OneLoneCoder.com - Command Line Game Engine

 A lot of what makes this possible are the Terminal Control Escape Sequences:
 http://www.termsys.demon.co.uk/vtansi.htm
*/

#pragma once

#include <cstdio>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <termios.h>
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <Carbon/Carbon.h>
#include <cmath>
#include <string>
#include <poll.h>
#include <chrono>
#include <vector>
#include <iomanip>
#include <codecvt>

enum COLOUR {
    RESET           = 0,

    FG_BLACK        = 30,
    FG_RED          = 31,
    FG_GREEN        = 32,
    FG_YELLOW       = 33,
    FG_BLUE         = 34,
    FG_MAGENTA      = 35,
    FG_CYAN         = 36,
    FG_WHITE        = 37,

    BG_BLACK        = 40,
    BG_RED          = 41,
    BG_GREEN        = 42,
    BG_YELLOW       = 43,
    BG_BLUE         = 44,
    BG_MAGENTA      = 45,
    BG_CYAN         = 46,
    BG_WHITE        = 47
};

enum PIXEL_TYPE
{
    PIXEL_SOLID = 0x2588,
    PIXEL_THREEQUARTERS = 0x2593,
    PIXEL_HALF = 0x2592,
    PIXEL_QUARTER = 0x2591
};

class olcConsoleGameEngine {

private:
    int m_nScreenWidth;
    int m_nScreenHeight;

    std::u16string* buffer;
    char** colors;

    char* blank;

    unsigned char keyMap[16];

    bool m_bAtomActive;

protected:
    std::wstring m_sAppName;

public:
    olcConsoleGameEngine()
    {
        m_nScreenWidth = 80;
        m_nScreenHeight = 30;

        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
            perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
            perror("tcsetattr ICANON");

        system("stty -echo"); //Disable key echo
        system("tput rmam"); //Disable Wrapping
        system("tput civis --invisible"); //Disable Cursor

        m_sAppName = L"Default";
    }

    int ConstructConsole(int width, int height, int fontsize) {
        m_nScreenWidth = width;
        m_nScreenHeight = height;

        buffer = new std::u16string[m_nScreenHeight];
        colors = new char*[m_nScreenHeight];
        for (int i = 0; i < m_nScreenHeight; ++i) {
            for (int j = 0; j < m_nScreenWidth; j++) buffer[i] += ' ';
            colors[i] = new char[m_nScreenWidth];
        }

        blank = new char[m_nScreenWidth];
        for (int j = 0; j < m_nScreenWidth; j++) blank[j] = 0;

        printf("\e[8;%d;%dt", m_nScreenHeight, m_nScreenWidth); //set width and height
        char buffer[90];
        sprintf(buffer, "osascript -e \"tell application \\\"Terminal\\\" to set the font size of window 1 to %d\"", fontsize); //set font size
        system(buffer);

        return true;
    }

    virtual void Draw(int x, int y, short c = PIXEL_SOLID, char col = 0)
    {
        if (x >= 0 && x < m_nScreenWidth && y >= 0 && y < m_nScreenHeight) {
            buffer[y][x] = c;
            colors[y][x] = col;
        }
    }

    void Fill(int x1, int y1, int x2, int y2, short c = PIXEL_SOLID, char col = 0)
    {
        Clip(x1, y1);
        Clip(x2, y2);
        for (int x = x1; x < x2; x++)
            for (int y = y1; y < y2; y++)
                Draw(x, y, c, col);
    }

    void DrawString(int x, int y, std::wstring c, char col = 0)
    {
        for (size_t i = 0; i < c.size(); i++)
        {
            buffer[y][x + i] = c[i];
            colors[y][x + i] = col;
        }
    }

    void DrawStringAlpha(int x, int y, std::wstring c, char col = 0)
    {
        for (size_t i = 0; i < c.size(); i++)
        {
            if (c[i] != L' ')
            {
                buffer[y][x + i] = c[i];
                colors[y][x + i] = col;
            }
        }
    }

    void Clip(int &x, int &y)
    {
        if (x < 0) x = 0;
        if (x >= m_nScreenWidth) x = m_nScreenWidth;
        if (y < 0) y = 0;
        if (y >= m_nScreenHeight) y = m_nScreenHeight;
    }

    void DrawLine(int x1, int y1, int x2, int y2, short c = 0x2588, char col = 0)
    {
        int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
        dx = x2 - x1; dy = y2 - y1;
        dx1 = abs(dx); dy1 = abs(dy);
        px = 2 * dy1 - dx1;    py = 2 * dx1 - dy1;
        if (dy1 <= dx1)
        {
            if (dx >= 0)
            { x = x1; y = y1; xe = x2; }
            else
            { x = x2; y = y2; xe = x1;}

            Draw(x, y, c, col);

            for (i = 0; x<xe; i++)
            {
                x = x + 1;
                if (px<0)
                    px = px + 2 * dy1;
                else
                {
                    if ((dx<0 && dy<0) || (dx>0 && dy>0)) y = y + 1; else y = y - 1;
                    px = px + 2 * (dy1 - dx1);
                }
                Draw(x, y, c, col);
            }
        }
        else
        {
            if (dy >= 0)
            { x = x1; y = y1; ye = y2; }
            else
            { x = x2; y = y2; ye = y1; }

            Draw(x, y, c, col);

            for (i = 0; y<ye; i++)
            {
                y = y + 1;
                if (py <= 0)
                    py = py + 2 * dx1;
                else
                {
                    if ((dx<0 && dy<0) || (dx>0 && dy>0)) x = x + 1; else x = x - 1;
                    py = py + 2 * (dx1 - dy1);
                }
                Draw(x, y, c, col);
            }
        }
    }

    void DrawTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c = 0x2588, char col = 0)
    {
        DrawLine(x1, y1, x2, y2, c, col);
        DrawLine(x2, y2, x3, y3, c, col);
        DrawLine(x3, y3, x1, y1, c, col);
    }

    // https://www.avrfreaks.net/sites/default/files/triangles.c
    void FillTriangle(int x1, int y1, int x2, int y2, int x3, int y3, short c = 0x2588, char col = 0)
    {
        auto SWAP = [](int &x, int &y) { int t = x; x = y; y = t; };
        auto drawline = [&](int sx, int ex, int ny) { for (int i = sx; i <= ex; i++) Draw(i, ny, c, col); };

        int t1x, t2x, y, minx, maxx, t1xp, t2xp;
        bool changed1 = false;
        bool changed2 = false;
        int signx1, signx2, dx1, dy1, dx2, dy2;
        int e1, e2;
        // Sort vertices
        if (y1>y2) { SWAP(y1, y2); SWAP(x1, x2); }
        if (y1>y3) { SWAP(y1, y3); SWAP(x1, x3); }
        if (y2>y3) { SWAP(y2, y3); SWAP(x2, x3); }

        t1x = t2x = x1; y = y1;   // Starting points
        dx1 = (int)(x2 - x1); if (dx1<0) { dx1 = -dx1; signx1 = -1; }
        else signx1 = 1;
        dy1 = (int)(y2 - y1);

        dx2 = (int)(x3 - x1); if (dx2<0) { dx2 = -dx2; signx2 = -1; }
        else signx2 = 1;
        dy2 = (int)(y3 - y1);

        if (dy1 > dx1) {   // swap values
            SWAP(dx1, dy1);
            changed1 = true;
        }
        if (dy2 > dx2) {   // swap values
            SWAP(dy2, dx2);
            changed2 = true;
        }

        e2 = (int)(dx2 >> 1);
        // Flat top, just process the second half
        if (y1 == y2) goto next;
        e1 = (int)(dx1 >> 1);

        for (int i = 0; i < dx1;) {
            t1xp = 0; t2xp = 0;
            if (t1x<t2x) { minx = t1x; maxx = t2x; }
            else { minx = t2x; maxx = t1x; }
            // process first line until y value is about to change
            while (i<dx1) {
                i++;
                e1 += dy1;
                while (e1 >= dx1) {
                    e1 -= dx1;
                    if (changed1) t1xp = signx1;//t1x += signx1;
                    else          goto next1;
                }
                if (changed1) break;
                else t1x += signx1;
            }
            // Move line
        next1:
            // process second line until y value is about to change
            while (1) {
                e2 += dy2;
                while (e2 >= dx2) {
                    e2 -= dx2;
                    if (changed2) t2xp = signx2;//t2x += signx2;
                    else          goto next2;
                }
                if (changed2)     break;
                else              t2x += signx2;
            }
        next2:
            if (minx>t1x) minx = t1x; if (minx>t2x) minx = t2x;
            if (maxx<t1x) maxx = t1x; if (maxx<t2x) maxx = t2x;
            drawline(minx, maxx, y);    // Draw line from min to max points found on the y
            // Now increase y
            if (!changed1) t1x += signx1;
            t1x += t1xp;
            if (!changed2) t2x += signx2;
            t2x += t2xp;
            y += 1;
            if (y == y2) break;

        }
    next:
        // Second half
        dx1 = (int)(x3 - x2); if (dx1<0) { dx1 = -dx1; signx1 = -1; }
        else signx1 = 1;
        dy1 = (int)(y3 - y2);
        t1x = x2;

        if (dy1 > dx1) {   // swap values
            SWAP(dy1, dx1);
            changed1 = true;
        }
        else changed1 = false;

        e1 = (int)(dx1 >> 1);

        for (int i = 0; i <= dx1; i++) {
            t1xp = 0; t2xp = 0;
            if (t1x<t2x) { minx = t1x; maxx = t2x; }
            else { minx = t2x; maxx = t1x; }
            // process first line until y value is about to change
            while (i<dx1) {
                e1 += dy1;
                while (e1 >= dx1) {
                    e1 -= dx1;
                    if (changed1) { t1xp = signx1; break; }//t1x += signx1;
                    else          goto next3;
                }
                if (changed1) break;
                else                t1x += signx1;
                if (i<dx1) i++;
            }
        next3:
            // process second line until y value is about to change
            while (t2x != x3) {
                e2 += dy2;
                while (e2 >= dx2) {
                    e2 -= dx2;
                    if (changed2) t2xp = signx2;
                    else          goto next4;
                }
                if (changed2)     break;
                else              t2x += signx2;
            }
        next4:

            if (minx>t1x) minx = t1x; if (minx>t2x) minx = t2x;
            if (maxx<t1x) maxx = t1x; if (maxx<t2x) maxx = t2x;
            drawline(minx, maxx, y);
            if (!changed1) t1x += signx1;
            t1x += t1xp;
            if (!changed2) t2x += signx2;
            t2x += t2xp;
            y += 1;
            if (y>y3) return;
        }
    }

    void DrawCircle(int xc, int yc, int r, short c = 0x2588, char col = 0)
    {
        int x = 0;
        int y = r;
        int p = 3 - 2 * r;
        if (!r) return;

        while (y >= x) // only formulate 1/8 of circle
        {
            Draw(xc - x, yc - y, c, col);//upper left left
            Draw(xc - y, yc - x, c, col);//upper upper left
            Draw(xc + y, yc - x, c, col);//upper upper right
            Draw(xc + x, yc - y, c, col);//upper right right
            Draw(xc - x, yc + y, c, col);//lower left left
            Draw(xc - y, yc + x, c, col);//lower lower left
            Draw(xc + y, yc + x, c, col);//lower lower right
            Draw(xc + x, yc + y, c, col);//lower right right
            if (p < 0) p += 4 * x++ + 6;
            else p += 4 * (x++ - y--) + 10;
        }
    }

    void FillCircle(int xc, int yc, int r, short c = 0x2588, char col = 0)
    {
        // Taken from wikipedia
        int x = 0;
        int y = r;
        int p = 3 - 2 * r;
        if (!r) return;

        auto drawline = [&](int sx, int ex, int ny)
        {
            for (int i = sx; i <= ex; i++)
                Draw(i, ny, c, col);
        };

        while (y >= x)
        {
            // Modified to draw scan-lines instead of edges
            drawline(xc - x, xc + x, yc - y);
            drawline(xc - y, xc + y, yc - x);
            drawline(xc - x, xc + x, yc + y);
            drawline(xc - y, xc + y, yc + x);
            if (p < 0) p += 4 * x++ + 6;
            else p += 4 * (x++ - y--) + 10;
        }
    };

    void DrawWireFrameModel(const std::vector<std::pair<float, float>> &vecModelCoordinates, float x, float y, float r = 0.0f, float s = 1.0f, char col = 0, short c = PIXEL_SOLID)
    {
        // pair.first = x coordinate
        // pair.second = y coordinate

        // Create translated model vector of coordinate pairs
        std::vector<std::pair<float, float>> vecTransformedCoordinates;
        int verts = (int)vecModelCoordinates.size();
        vecTransformedCoordinates.resize(verts);

        // Rotate
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecModelCoordinates[i].first * cosf(r) - vecModelCoordinates[i].second * sinf(r);
            vecTransformedCoordinates[i].second = vecModelCoordinates[i].first * sinf(r) + vecModelCoordinates[i].second * cosf(r);
        }

        // Scale
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first * s;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second * s;
        }

        // Translate
        for (int i = 0; i < verts; i++)
        {
            vecTransformedCoordinates[i].first = vecTransformedCoordinates[i].first + x;
            vecTransformedCoordinates[i].second = vecTransformedCoordinates[i].second + y;
        }

        // Draw Closed Polygon
        for (int i = 0; i < verts + 1; i++)
        {
            int j = (i + 1);
            DrawLine((int)vecTransformedCoordinates[i % verts].first, (int)vecTransformedCoordinates[i % verts].second,
                     (int)vecTransformedCoordinates[j % verts].first, (int)vecTransformedCoordinates[j % verts].second, c, col);
        }
    }

    ~olcConsoleGameEngine()
    {
        for (int i = 0; i < m_nScreenHeight; i++) {
            delete[] colors[i];
        }
        delete[] buffer;
        delete[] colors;
        delete[] blank;


        setColor(0);
        system("stty echo");
        resetTerm();
    }

public:
    void Start()
    {
        // Start the thread
        m_bAtomActive = true;
        GameThread();
    }

    int ScreenWidth()
    {
        return m_nScreenWidth;
    }

    int ScreenHeight()
    {
        return m_nScreenHeight;
    }

private:
    void pollKeys() {
        GetKeys((BigEndianUInt32*) &keyMap);
    }

    void resetTerm() {
        system("printf '\033c'");
    }

    void goToLine(int column, int row) {
        printf("\e[%d;%dH", row, column);
    }

    void setColor(int fore, int back) {
        printf("\e[%d;%dm", fore, back);
    }

    void setColor(int fore) {
        printf("\e[%dm", fore);
    }

    void GameThread()
    {
        // Create user resources as part of this thread
        if (!OnUserCreate())
            m_bAtomActive = false;

        auto tp1 = std::chrono::system_clock::now();
        auto tp2 = std::chrono::system_clock::now();

        std::wstring_convert<std::codecvt_utf8_utf16<char16_t>,char16_t> codecvt;

        while (m_bAtomActive)
        {
            // Run as fast as possible
            while (m_bAtomActive)
            {
                // Handle Timing
                tp2 = std::chrono::system_clock::now();
                std::chrono::duration<float> elapsedTime = tp2 - tp1;
                tp1 = tp2;
                float fElapsedTime = elapsedTime.count();

                for (int i = 0; i < m_nScreenHeight; i++) {
                    for (int j = 0; j < m_nScreenWidth; j++) {
                        buffer[i][j] = L' ';
                        colors[i][j] = 0;
                    }
                }

                pollKeys();

                // Handle Frame Update
                if (!OnUserUpdate(fElapsedTime))
                    m_bAtomActive = false;

                // Update Title & Present Screen Buffer
                std::wcout << L"\033]0;OneLoneCoder.com - Console Game Engine - " << m_sAppName << L" - FPS: " << std::setprecision(4) << 1.0f / fElapsedTime << L"\007";

                goToLine(0, 0);
                for (int i = 0; i < m_nScreenHeight; i++) {
                    if (memcmp(blank, colors[i], m_nScreenWidth)) {
                        for (int j = 0; j < m_nScreenWidth; j++) {
                            if (colors[i][j]) setColor(colors[i][j]);
                            else setColor(37, 40);
                            std::cout << codecvt.to_bytes(buffer[i][j]);
                        }
                    }
                    else {
                        setColor(37, 40);
                        std::cout << codecvt.to_bytes(buffer[i]);
                    }
                    if (i < m_nScreenHeight - 1) std::cout << std::endl;
                }
            }

            // Allow the user to free resources if they have overrided the destroy function
            if (OnUserDestroy())
            {
                // User has permitted destroy, so exit and clean up
            }
            else
            {
                // User denied destroy for some reason, so continue running
                m_bAtomActive = true;
            }
        }
    }

public:
    // User MUST OVERRIDE THESE!!
    virtual bool OnUserCreate()                            = 0;
    virtual bool OnUserUpdate(float fElapsedTime)          = 0;

    // Optional for clean up
    virtual bool OnUserDestroy()                        { return true; }

public:
    bool GetKey(int inKeyCode){ return (0 != ((keyMap[ inKeyCode >> 3] >> (inKeyCode & 7)) & 1)); }
};
