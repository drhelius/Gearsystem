/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013  Ignacio Sanchez

 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/
 *
 */

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl.h>
#else
#include <GL/glew.h>
#include <SDL_opengl.h>
#endif

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "emu.h"
#include "config.h"
#include "../../src/gearsystem.h"

#define RENDERER_IMPORT
#include "renderer.h"

static uint32_t fbo_texture[3];
static uint32_t system_texture[3];
static uint32_t scanlines_texture;
static uint32_t current_system_texture;
static uint32_t frame_buffer_object[3];
static uint32_t current_fbo;
static GS_RuntimeInfo current_runtime;
static bool first_frame;
static u32 scanlines[16] = {0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000000FF, 0x000000FF, 0x000000FF, 0x000000FF};
static const int FRAME_BUFFER_SCALE = 4;

static void init_ogl_gui(void);
static void init_ogl_emu(void);
static void init_ogl_debug(void);
static void init_texture(int index, int w, int h);
static void init_scanlines_texture(void);
static void render_gui(void);
static void render_emu_normal(void);
static void render_emu_mix(void);
static void render_emu_bilinear(void);
static void render_quad(int viewportWidth, int viewportHeight);
static void update_system_texture(void);
static void update_debug_textures(void);
static void render_scanlines(void);

void renderer_init(void)
{
    #ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        Log("GLEW Error: %s\n", glewGetErrorString(err));
    }

    renderer_glew_version = (const char*)glewGetString(GLEW_VERSION);
    renderer_opengl_version = (const char*)glGetString(GL_VERSION);

    Log("Using GLEW %s\n", renderer_glew_version);
    
    #endif

    init_ogl_gui();
    init_ogl_emu();
    init_ogl_debug();

    first_frame = true;
}

void renderer_destroy(void)
{
    glDeleteFramebuffers(3, frame_buffer_object); 
    glDeleteTextures(3, fbo_texture);
    glDeleteTextures(3, system_texture);
    glDeleteTextures(1, &scanlines_texture);
    glDeleteTextures(1, &renderer_emu_debug_vram_background);
    glDeleteTextures(1, &renderer_emu_debug_vram_tiles);
    glDeleteTextures(64, renderer_emu_debug_vram_sprites);
    ImGui_ImplOpenGL2_Shutdown();
}

void renderer_begin_render(void)
{
    ImGui_ImplOpenGL2_NewFrame();
}

void renderer_render(void)
{
    emu_get_runtime(current_runtime);

    int res = 0;

    switch (current_runtime.screen_height)
    {
        case GS_RESOLUTION_GG_HEIGHT:
            res = 0;
            break;
        case GS_RESOLUTION_SMS_HEIGHT:
            res = 1;
            break;
        case GS_RESOLUTION_SMS_HEIGHT_EXTENDED:
            res = 2;
            break;
    }

    current_fbo = frame_buffer_object[res];
    current_system_texture = system_texture[res];
    renderer_emu_texture = fbo_texture[res];

    if (config_debug.debug)
    {
        update_debug_textures();
    }

    if (config_video.mix_frames)
        render_emu_mix();
    else
        render_emu_normal();

    if (config_video.scanlines)
        render_scanlines();

    render_emu_bilinear();

    ImVec4 clear_color = ImVec4(0.1f, 0.1f, 0.1f, 1.00f);

    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);

    render_gui();
}

void renderer_end_render(void)
{

}

static void init_ogl_gui(void)
{
    ImGui_ImplOpenGL2_Init();
}

static void init_ogl_emu(void)
{
    glEnable(GL_TEXTURE_2D);

    glGenFramebuffers(3, frame_buffer_object);
    glGenTextures(3, fbo_texture);
    glGenTextures(3, system_texture);

    init_texture(0, GS_RESOLUTION_GG_WIDTH, GS_RESOLUTION_GG_HEIGHT);
    init_texture(1, GS_RESOLUTION_SMS_WIDTH, GS_RESOLUTION_SMS_HEIGHT);
    init_texture(2, GS_RESOLUTION_SMS_WIDTH, GS_RESOLUTION_SMS_HEIGHT_EXTENDED);

    init_scanlines_texture();
}

static void init_ogl_debug(void)
{
    glGenTextures(1, &renderer_emu_debug_vram_background);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_background);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_background_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glGenTextures(1, &renderer_emu_debug_vram_tiles);
    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_tiles);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 32 * 8, 32 * 8, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_tile_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    for (int s = 0; s < 64; s++)
    {
        glGenTextures(1, &renderer_emu_debug_vram_sprites[s]);
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_sprites[s]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8, 16, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*)emu_debug_sprite_buffers[s]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void init_texture(int index, int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_object[index]);
    glBindTexture(GL_TEXTURE_2D, fbo_texture[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w * FRAME_BUFFER_SCALE, h * FRAME_BUFFER_SCALE, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo_texture[index], 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindTexture(GL_TEXTURE_2D, system_texture[index]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

static void init_scanlines_texture(void)
{
    glGenTextures(1, &scanlines_texture);
    glBindTexture(GL_TEXTURE_2D, scanlines_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8, (GLvoid*) scanlines);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
}

static void render_gui(void)
{
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
}

static void render_emu_normal(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);

    glDisable(GL_BLEND);

    update_system_texture();

    render_quad(current_runtime.screen_width * FRAME_BUFFER_SCALE, current_runtime.screen_height * FRAME_BUFFER_SCALE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void render_emu_mix(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);

    float alpha = 0.15f + (0.20f * (1.0f - config_video.mix_frames_intensity));

    if (first_frame)
    {
        first_frame = false;
        alpha = 1.0f;
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }

    static bool round_error = false; 
    float round_color = 1.0f - (round_error ? 0.03f : 0.0f);
    round_error = !round_error;

    glEnable(GL_BLEND);
    glColor4f(round_color, round_color, round_color, alpha);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    update_system_texture();

    render_quad(current_runtime.screen_width * FRAME_BUFFER_SCALE, current_runtime.screen_height * FRAME_BUFFER_SCALE);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

static void update_system_texture(void)
{
    glBindTexture(GL_TEXTURE_2D, current_system_texture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, current_runtime.screen_width, current_runtime.screen_height,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_frame_buffer);

    if (config_video.bilinear)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void update_debug_textures(void)
{
    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_background);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256,
            GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_background_buffer);

    glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_tiles);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 32 * 8, 32 * 8,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_tile_buffer);

    for (int s = 0; s < 64; s++)
    {
        glBindTexture(GL_TEXTURE_2D, renderer_emu_debug_vram_sprites[s]);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 16,
                GL_RGB, GL_UNSIGNED_BYTE, (GLvoid*) emu_debug_sprite_buffers[s]);
    }
}

static void render_emu_bilinear(void)
{
    glBindTexture(GL_TEXTURE_2D, renderer_emu_texture);

    if (config_video.scanlines)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
}

static void render_quad(int viewportWidth, int viewportHeight)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(1.0, 0.0);
    glVertex2d(viewportWidth, 0.0);
    glTexCoord2d(1.0, 1.0);
    glVertex2d(viewportWidth, viewportHeight);
    glTexCoord2d(0.0, 1.0);
    glVertex2d(0.0, viewportHeight);
    glEnd();
}

static void render_scanlines(void)
{
    glBindFramebuffer(GL_FRAMEBUFFER, current_fbo);
    glEnable(GL_BLEND);

    glColor4f(1.0f, 1.0f, 1.0f, config_video.scanlines_intensity / 4.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, scanlines_texture);

    int viewportWidth = current_runtime.screen_width * FRAME_BUFFER_SCALE;
    int viewportHeight = current_runtime.screen_height * FRAME_BUFFER_SCALE;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, viewportWidth, 0, viewportHeight, -1, 1);

    glMatrixMode(GL_MODELVIEW);
    glViewport(0, 0, viewportWidth, viewportHeight);

    glBegin(GL_QUADS);
    glTexCoord2d(0.0, 0.0);
    glVertex2d(0.0, 0.0);
    glTexCoord2d(current_runtime.screen_width, 0.0);
    glVertex2d(viewportWidth, 0.0);
    glTexCoord2d(current_runtime.screen_width, current_runtime.screen_height);
    glVertex2d(viewportWidth, viewportHeight);
    glTexCoord2d(0.0, current_runtime.screen_height);
    glVertex2d(0.0, viewportHeight);
    glEnd();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
