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
#include "scene.h"

#define WIDTH 800
#define HEIGHT 600

Surface g_surf = { WIDTH, HEIGHT, WIDTH*4, NULL };
static HBITMAP g_backBmp = NULL;
static HDC     g_memDC   = NULL;

static inline void ndc_to_screen(float x_ndc, float y_ndc, int W, int H, float* outx, float* outy){
    *outx = (x_ndc * 0.5f + 0.5f) * (float)W;
    *outy = (1.0f - (y_ndc * 0.5f + 0.5f)) * (float)H;
}


void renderer_draw_object(Surface* s, const SceneObject* object, const Mat4* V, const Mat4* P) {
    Mat4 M = transform_get_matrix(&object->transform);
    Mat4 VM = mat4_mul(*V, M);
    Mat4 PVM = mat4_mul(*P, VM);

    int num_verts = object->mesh->num_vertices;
    Vec3* world_space_normals = (Vec3*)malloc(num_verts * sizeof(Vec3));
    Vec4* clip_space_verts = (Vec4*)malloc(num_verts * sizeof(Vec4));
    float* sx = (float*)malloc(num_verts * sizeof(float));
    float* sy = (float*)malloc(num_verts * sizeof(float));
    float* z_over_w = (float*)malloc(num_verts * sizeof(float));
    float* q = (float*)malloc(num_verts * sizeof(float));

    for (int i = 0; i < num_verts; ++i) {
        Vec4 v_model = vec4(object->mesh->vertices[i].pos.x, object->mesh->vertices[i].pos.y, object->mesh->vertices[i].pos.z, 1.0f);
        Vec3 n_model = object->mesh->vertices[i].normal;
        
        world_space_normals[i] = v3_norm(mat3_mul_v3(M, n_model));

        clip_space_verts[i] = mat4_mul_v4(PVM, v_model);

        q[i] = 1.0f / clip_space_verts[i].w;
        ndc_to_screen(clip_space_verts[i].x * q[i], clip_space_verts[i].y * q[i], s->width, s->height, &sx[i], &sy[i]);
        z_over_w[i] = clip_space_verts[i].z * q[i];
    }

    Vec3 light_direction = v3_norm((Vec3){0.5f, 1.0f, -0.75f});
    const float AMBIENT = 0.2f;
    const float DIFFUSE = 0.8f;
    Color32 base_color = object->base_color;

    for (int f = 0; f < object->mesh->num_faces; ++f) {
        int i0 = object->mesh->indices[f * 3 + 0];
        int i1 = object->mesh->indices[f * 3 + 1];
        int i2 = object->mesh->indices[f * 3 + 2];

        // float signed_area = (sx[i1] - sx[i0]) * (sy[i2] - sy[i0]) - (sx[i2] - sx[i0]) * (sy[i1] - sy[i0]);
        // if (signed_area >= 0) continue;
        
        if (clip_space_verts[i0].w <= 0.1f || clip_space_verts[i1].w <= 0.1f || clip_space_verts[i2].w <= 0.1f) continue;
        
        float shade0 = AMBIENT + DIFFUSE * fmaxf(0.0f, v3_dot(world_space_normals[i0], vec3(-light_direction.x, -light_direction.y, -light_direction.z)));
        float shade1 = AMBIENT + DIFFUSE * fmaxf(0.0f, v3_dot(world_space_normals[i1], vec3(-light_direction.x, -light_direction.y, -light_direction.z)));
        float shade2 = AMBIENT + DIFFUSE * fmaxf(0.0f, v3_dot(world_space_normals[i2], vec3(-light_direction.x, -light_direction.y, -light_direction.z)));

        Color32 c0 = color_scale(base_color, shade0);
        Color32 c1 = color_scale(base_color, shade1);
        Color32 c2 = color_scale(base_color, shade2);

        float r0ow = c0.r * q[i0], g0ow = c0.g * q[i0], b0ow = c0.b * q[i0];
        float r1ow = c1.r * q[i1], g1ow = c1.g * q[i1], b1ow = c1.b * q[i1];
        float r2ow = c2.r * q[i2], g2ow = c2.g * q[i2], b2ow = c2.b * q[i2];

        tri_fill_persp_gouraud(s,
            sx[i0], sy[i0], q[i0], z_over_w[i0], r0ow, g0ow, b0ow,
            sx[i1], sy[i1], q[i1], z_over_w[i1], r1ow, g1ow, b1ow,
            sx[i2], sy[i2], q[i2], z_over_w[i2], r2ow, g2ow, b2ow
        );
    }
    
    free(world_space_normals);
    free(clip_space_verts);
    free(sx); free(sy); free(z_over_w); free(q);
}

SceneObject g_cubes[2];
Mesh* g_cube_mesh = NULL;


void init_scene() {
    g_cube_mesh = mesh_create_cube();

    g_cubes[0].mesh = g_cube_mesh;
    g_cubes[0].transform.position = (Vec3){-1.5f, 0.0f, 0.0f};
    g_cubes[0].transform.rotation = (Vec3){0.0f, 0.0f, 0.0f};
    g_cubes[0].transform.scale = (Vec3){1.0f, 1.0f, 1.0f};
    g_cubes[0].base_color = RGB8(60, 150, 220); 

    g_cubes[1].mesh = g_cube_mesh;
    g_cubes[1].transform.position = (Vec3){1.5f, 0.0f, 0.0f};
    g_cubes[1].transform.rotation = (Vec3){0.0f, 0.0f, 0.0f};
    g_cubes[1].transform.scale = (Vec3){0.7f, 0.7f, 0.7f};
    g_cubes[1].base_color = RGB8(220, 90, 60);
}

void render_the_scene(Surface* s, float time_sec) {
    clear(s, RGB8(20, 20, 24));
    depth_clear(s, 1e30f);

    g_cubes[0].transform.rotation.y = time_sec * 1.0f;
    g_cubes[0].transform.rotation.x = time_sec * 0.5f;

    g_cubes[1].transform.rotation.y = -time_sec * 0.8f;
    
    float aspect = (float)s->width / (float)s->height;
    Mat4 P = mat4_perspective(60.0f * (3.14159265f / 180.0f), aspect, 0.1f, 100.0f);
    Mat4 V = mat4_translation(0, 0, -7); 

    renderer_draw_object(s, &g_cubes[0], &V, &P);
    renderer_draw_object(s, &g_cubes[1], &V, &P);
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

    init_scene();

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

        render_the_scene(&g_surf, (float)t);
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

