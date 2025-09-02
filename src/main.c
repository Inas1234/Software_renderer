#ifndef UNICODE
#define UNICODE
#endif

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"

#include "windows.h"
#include "surface.h"
#include "draw.h"
#include "color.h"
#include "mathx.h"
#define WIDTH 800
#define HEIGHT 600

Surface g_surf = { WIDTH, HEIGHT, WIDTH*4, NULL };
static HBITMAP g_backBmp = NULL;
static HDC     g_memDC   = NULL;

typedef struct { Vec3 pos; } Vtx;

static const Vtx CUBE_V[8] = {
    {{-1,-1,-1}}, {{ 1,-1,-1}}, {{ 1, 1,-1}}, {{-1, 1,-1}}, 
    {{-1,-1, 1}}, {{ 1,-1, 1}}, {{ 1, 1, 1}}, {{-1, 1, 1}}, 
};

static const int CUBE_TRI[12][3] = {
    {0,1,2},{0,2,3}, 
    {4,6,5},{4,7,6}, 
    {0,4,5},{0,5,1}, 
    {3,2,6},{3,6,7}, 
    {1,5,6},{1,6,2}, 
    {0,3,7},{0,7,4}, 
};

static const Color32 FACE_COL[6] = {
    RGB8(220, 60, 60), RGB8(60, 220, 60), RGB8(60, 60, 220),
    RGB8(220, 180, 60), RGB8(60, 200, 200), RGB8(200, 60, 200)
};

static inline void ndc_to_screen(float x_ndc, float y_ndc, int W, int H, float* outx, float* outy){
    *outx = (x_ndc * 0.5f + 0.5f) * (float)W;
    *outy = (1.0f - (y_ndc * 0.5f + 0.5f)) * (float)H; 
}

void render_cube(Surface* s, float time_sec){
    clear(s, RGB8(20,20,24));
    depth_clear(s, 1e30f);

    float aspect = (float)s->width / (float)s->height;
    Mat4 P = mat4_perspective(60.0f*(3.14159265f/180.0f), aspect, 0.1f, 100.0f);
    Mat4 V = mat4_translation(0,0,-5);        
    Mat4 M = mat4_rotation_y(time_sec*1.0f);  
    Mat4 MVP = mat4_mul(P, mat4_mul(V, M));

    Vec4 clip[8];
    float sx[8], sy[8], z_over_w[8], q[8];
    for (int i=0;i<8;++i){
        Vec4 v = vec4(CUBE_V[i].pos.x, CUBE_V[i].pos.y, CUBE_V[i].pos.z, 1.0f);
        Vec4 c = mat4_mul_v4(MVP, v);
        clip[i] = c;
        float invw = 1.0f / c.w;
        float x_ndc = c.x * invw;
        float y_ndc = c.y * invw;
        float z_ndc = c.z * invw;  
        q[i] = invw;
        z_over_w[i] = z_ndc;
        ndc_to_screen(x_ndc, y_ndc, s->width, s->height, &sx[i], &sy[i]);
    }

    for (int f=0; f<12; ++f){
        int i0 = CUBE_TRI[f][0], i1 = CUBE_TRI[f][1], i2 = CUBE_TRI[f][2];
        Color32 col = FACE_COL[f/2];

        tri_fill_persp_z(
            s,
            sx[i0], sy[i0], z_over_w[i0], q[i0],
            sx[i1], sy[i1], z_over_w[i1], q[i1],
            sx[i2], sy[i2], z_over_w[i2], q[i2],
            col
        );
    }
}

static void init_bmi(void) {
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = WIDTH;
    bmi.bmiHeader.biHeight      = -HEIGHT;      
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;           
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = NULL;
    g_backBmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    g_memDC   = CreateCompatibleDC(NULL);
    SelectObject(g_memDC, g_backBmp);
    g_surf.pixels = (Color32*)bits;
    g_surf.depth = (float*)malloc(g_surf.width * g_surf.height * sizeof(float));
    depth_clear(&g_surf, 1e30f);
}

void render_scene(void){
    clear(&g_surf, RGB8(20,20,24));
    depth_clear(&g_surf, 1e30f); 

    tri_fill_z_color(&g_surf, 120.0f, 60.0f,  5.0f, RGB8(255,0,0),    
                               420.0f, 220.0f, 5.0f, RGB8(0,255,0),   
                                60.0f, 300.0f, 5.0f, RGB8(0,0,255));  

    tri_fill_z_color(&g_surf, 160.0f, 100.0f, 1.0f, RGB8(255,255,0),  
                               460.0f, 260.0f, 1.0f, RGB8(0,255,255), 
                               120.0f, 340.0f, 1.0f, RGB8(255,0,255));
}

static void present(HDC hdc, int dstW, int dstH) {
    if (dstW <= 0 || dstH <= 0) return;
    if (dstW == WIDTH && dstH == HEIGHT) {
        BitBlt(hdc, 0, 0, WIDTH, HEIGHT, g_memDC, 0, 0, SRCCOPY);
    } else {
        SetStretchBltMode(hdc, COLORONCOLOR);
        StretchBlt(hdc, 0, 0, dstW, dstH, g_memDC, 0, 0, WIDTH, HEIGHT, SRCCOPY);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    const wchar_t CLASS_NAME[]  = L"Graphics engine";
    
    WNDCLASS wc = { };

    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);


    HWND hwnd = CreateWindowEx(
        0,                             
        CLASS_NAME,                     
        L"Graphics engine",   
        WS_OVERLAPPEDWINDOW,            

        CW_USEDEFAULT, CW_USEDEFAULT, WIDTH, HEIGHT,

        NULL,       
        NULL,
        hInstance,  
        NULL        
        );

    if (hwnd == NULL)
    {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);                


    LARGE_INTEGER freq, prev; QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&prev);
    double acc = 0.0, t = 0.0; const double dt = 1.0/60.0;
    int frames = 0; double fps_time = 0.0;

    BOOL running = TRUE;
    while (running) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) { running = FALSE; break; }
            TranslateMessage(&msg); DispatchMessage(&msg);
        }
        LARGE_INTEGER now; QueryPerformanceCounter(&now);
        double frame = (double)(now.QuadPart - prev.QuadPart) / (double)freq.QuadPart;
        prev = now;
        if (frame > 0.25) frame = 0.25; 

        acc += frame; fps_time += frame;

        while (acc >= dt) {
            acc -= dt; t += dt;
        }

        render_cube(&g_surf, (float)t);
        // render_scene();                 
        InvalidateRect(hwnd, NULL, FALSE);

        frames++;
        if (fps_time >= 0.5) {
            double fps = frames / fps_time;
            wchar_t title[128];
            swprintf(title, 128, L"Graphics engine  |  %.1f FPS", fps);
            SetWindowText(hwnd, title);
            frames = 0; fps_time = 0.0;
        }
    }


    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_CREATE:
        init_bmi();
        render_scene();
        InvalidateRect(hwnd, NULL, FALSE); 
        return 0;

    case WM_SIZE:
        InvalidateRect(hwnd, NULL, FALSE);
        return 0;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rc; GetClientRect(hwnd, &rc);
            int w = rc.right - rc.left, h = rc.bottom - rc.top;
            present(hdc, w, h);

            EndPaint(hwnd, &ps);
        }
        return 0;

    case WM_DESTROY:
        if (g_memDC) { DeleteDC(g_memDC); g_memDC=NULL; }
        if (g_backBmp){ DeleteObject(g_backBmp); g_backBmp=NULL; }
        if (g_surf.depth) { free(g_surf.depth); g_surf.depth = NULL; }
        PostQuitMessage(0);
        return 0;

    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

