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
    // --- Matrix Setup ---
    Mat4 M = transform_get_matrix(&object->transform);
    Mat4 V_M = mat4_mul(*V, M);   // Renamed from MV for clarity
    Mat4 P_VM = mat4_mul(*P, V_M); // Renamed from MVP

    // --- Vertex Transformation ---
    int num_verts = object->mesh->num_vertices;
    // We now need vertices in WORLD space for lighting, and CLIP space for rasterization.
    Vec3* world_space_verts = (Vec3*)malloc(num_verts * sizeof(Vec3));
    Vec4* clip_space_verts = (Vec4*)malloc(num_verts * sizeof(Vec4));
    float* sx = (float*)malloc(num_verts * sizeof(float));
    float* sy = (float*)malloc(num_verts * sizeof(float));
    float* z_over_w = (float*)malloc(num_verts * sizeof(float));
    float* q = (float*)malloc(num_verts * sizeof(float));

    for (int i = 0; i < num_verts; ++i) {
        Vec4 vM = vec4(object->mesh->vertices[i].pos.x, object->mesh->vertices[i].pos.y, object->mesh->vertices[i].pos.z, 1.0f);
        
        // 1. Transform to World Space (for lighting)
        Vec4 v_world_4 = mat4_mul_v4(M, vM);
        world_space_verts[i] = (Vec3){v_world_4.x, v_world_4.y, v_world_4.z};
        
        // 2. Transform to Clip Space (for rasterization)
        Vec4 v_clip = mat4_mul_v4(P_VM, vM);
        clip_space_verts[i] = v_clip;

        // 3. Perspective divide and map to screen
        float invw = 1.0f / v_clip.w;
        ndc_to_screen(v_clip.x * invw, v_clip.y * invw, s->width, s->height, &sx[i], &sy[i]);
        z_over_w[i] = v_clip.z * invw;
        q[i] = invw;
    }

    // --- Lighting Setup (in World Space) ---
    // The direction the light rays are traveling.
    Vec3 light_direction = v3_norm((Vec3){0.4f, 0.8f, -0.6f});
    const float AMBIENT = 0.15f;
    const float DIFFUSE = 0.85f;

    // --- Triangle Processing ---
    for (int f = 0; f < object->mesh->num_faces; ++f) {
        int i0_idx = f * 3 + 0;
        int i1_idx = f * 3 + 1;
        int i2_idx = f * 3 + 2;
        
        int i0 = object->mesh->indices[i0_idx];
        int i1 = object->mesh->indices[i1_idx];
        int i2 = object->mesh->indices[i2_idx];

        // Backface Culling in Clip Space (faster and simpler)
        float signed_area = (sx[i1] - sx[i0]) * (sy[i2] - sy[i0]) - (sx[i2] - sx[i0]) * (sy[i1] - sy[i0]);
        if (signed_area >= 0) continue;

        // --- Lighting Calculation (now in World Space) ---
        Vec3 p0_world = world_space_verts[i0];
        Vec3 p1_world = world_space_verts[i1];
        Vec3 p2_world = world_space_verts[i2];
        
        Vec3 n_world = v3_cross(v3_sub(p1_world, p0_world), v3_sub(p2_world, p0_world));
        Vec3 n_world_norm = v3_norm(n_world);
        
        // We need the vector FROM the surface TO the light, so we negate the light's direction vector.
        float ndotl = v3_dot(n_world_norm, vec3(-light_direction.x, -light_direction.y, -light_direction.z));
        
        if (ndotl < 0.0f) ndotl = 0.0f; // Light doesn't illuminate from behind
        float shade = AMBIENT + DIFFUSE * ndotl;

        Color32 base = object->mesh->face_colors[f/2];
        Color32 col = color_scale(base, shade);

        // Simple clipping check
        if (clip_space_verts[i0].w <= 0.1f || clip_space_verts[i1].w <= 0.1f || clip_space_verts[i2].w <= 0.1f) continue;

        tri_fill_persp_z(s,
            sx[i0], sy[i0], z_over_w[i0], q[i0],
            sx[i1], sy[i1], z_over_w[i1], q[i1],
            sx[i2], sy[i2], z_over_w[i2], q[i2],
            col);
    }
    
    // Free allocated memory
    free(world_space_verts);
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

    g_cubes[1].mesh = g_cube_mesh;
    g_cubes[1].transform.position = (Vec3){1.5f, 0.0f, 0.0f};
    g_cubes[1].transform.rotation = (Vec3){0.0f, 0.0f, 0.0f};
    g_cubes[1].transform.scale = (Vec3){0.7f, 0.7f, 0.7f}; 
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

