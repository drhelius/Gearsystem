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

#import <GLKit/GLKit.h>
#import "Emulator.h"
#include <SDL2/SDL.h>
#include "inputmanager.h"

@implementation Emulator

@synthesize multiplier, retina, iPad, theGearsystemCore;

-(id)init
{
    if (self = [super init])
    {
        scr_w = 256;
        scr_h = 192;
        saveStatePending = NO;
        loadStatePending = NO;
        
#ifdef __APPLE__
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
        SDL_SetMainReady();
#endif
#endif
        
        theGearsystemCore = new GearsystemCore();
        theGearsystemCore->Init();

        theSoundQueue = new Sound_Queue();
        theSoundQueue->start(44100, 2);

        audioEnabled = YES;
        
        theInput = new EmulatorInput(self);
        theInput->Init();
        theFrameBuffer = new GS_Color[GS_RESOLUTION_MAX_WIDTH * GS_RESOLUTION_MAX_HEIGHT];
        theTexture = new GS_Color[256 * 256];
        
        for (int y = 0; y < GS_RESOLUTION_MAX_HEIGHT; ++y)
        {
            for (int x = 0; x < GS_RESOLUTION_MAX_WIDTH; ++x)
            {
                int pixel = (y * GS_RESOLUTION_MAX_WIDTH) + x;
                theFrameBuffer[pixel].red = theFrameBuffer[pixel].green = theFrameBuffer[pixel].blue = 0x00;
                theFrameBuffer[pixel].alpha = 0xFF;
            }
        }

        for (int y = 0; y < 256; ++y)
        {
            for (int x = 0; x < 256; ++x)
            {
                int pixel = (y * 256) + x;
                theTexture[pixel].red = theTexture[pixel].green = theTexture[pixel].blue = 0x00;
                theTexture[pixel].alpha = 0xFF;
            }
        }
    }
    return self;
}

-(void)dealloc
{
    theGearsystemCore->SaveRam();
    SafeDeleteArray(theTexture);
    SafeDeleteArray(theFrameBuffer);
    SafeDelete(theSoundQueue);
    SafeDelete(theGearsystemCore);
    [self shutdownGL];
}

-(void)shutdownGL
{
    TextureManager::Instance().UnloadAll();
    glDeleteTextures(1, &GBTexture);
}

-(void)update
{
    if (saveStatePending)
    {
        saveStatePending = NO;
        theGearsystemCore->SaveState(1);
    }
    else if (loadStatePending)
    {
        loadStatePending = NO;
        theGearsystemCore->LoadState(1);
    }
    
    InputManager::Instance().Update();

    int sampleCount = 0;

    theGearsystemCore->RunToVBlank(theFrameBuffer, theSampleBufffer, &sampleCount);
    
    GS_RuntimeInfo runtime_info;
    if (theGearsystemCore->GetRuntimeInfo(runtime_info))
    {
        scr_w = runtime_info.screen_width;
        scr_h = runtime_info.screen_height;
    }

    if (audioEnabled && (sampleCount > 0))
    {
        theSoundQueue->write(theSampleBufffer, sampleCount);
    }

    for (int y = 0; y < scr_h; ++y)
    {
        int y_256 = y * 256;
        int y_gb_width = y * scr_w;
        for (int x = 0; x < scr_w; ++x)
        {
            theTexture[y_256 + x] = theFrameBuffer[y_gb_width + x];
        }
    }
}

-(void)draw
{
    [self renderFrame];
}

-(void)initGL
{
    glGenTextures(1, &GBTexture);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, GBTexture);
    [self setupTextureWithData: (GLvoid*) theTexture];
    
    scanlineTexture = TextureManager::Instance().GetTexture("/scanlines2x");
}

-(void)renderFrame
{
    glBindTexture(GL_TEXTURE_2D, GBTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 256, 256, GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) theTexture);
    [self renderQuadWithViewportWidth:(scr_w) andHeight:(scr_h) andMirrorY:NO];
    
    if (IsValidPointer(scanlineTexture))
    {
        glBindTexture(GL_TEXTURE_2D, scanlineTexture->GetID());
        glEnable(GL_BLEND);
        glColor4f(1.0f, 1.0f, 1.0f, 0.35f);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        [self renderQuadWithViewportWidth:(scr_w) andHeight:(scr_h) andMirrorY:NO];
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glDisable(GL_BLEND);
    }
}

-(void)setupTextureWithData: (GLvoid*) data
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
}

-(void)renderQuadWithViewportWidth: (float)viewportWidth andHeight: (float)viewportHeight andMirrorY: (BOOL) mirrorY
{
    float kGB_Width = viewportWidth;
    float kGB_Height = viewportHeight;
    float kGB_TexWidth = kGB_Width / 256.0f;
    float kGB_TexHeight = kGB_Height / 256.0f;
    GLfloat box[] = {0.0f, kGB_Height, 1.0f, kGB_Width,kGB_Height, 1.0f, 0.0f, 0.0f, 1.0f, kGB_Width, 0.0f, 1.0f};
    GLfloat tex[] = {0.0f, kGB_TexHeight, kGB_TexWidth, kGB_TexHeight, 0.0f, 0.0f, kGB_TexWidth, 0.0f};
    
    glVertexPointer(3, GL_FLOAT, 0, box);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (mirrorY)
        glOrthof(0.0f, kGB_Width, 0.0f, kGB_Height, -1.0f, 1.0f);
    else
        glOrthof(0.0f, kGB_Width, kGB_Height, 0.0f, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, 128 * multiplier, 92 * multiplier);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

-(void)loadRomWithPath: (NSString *)filePath
{
    theGearsystemCore->SaveRam();
    theGearsystemCore->LoadROM([filePath UTF8String]);
    theGearsystemCore->LoadRam();
}

-(void)keyPressed: (GS_Keys)key
{
    theGearsystemCore->KeyPressed(Joypad_1, key);
}

-(void)keyReleased: (GS_Keys)key
{
    theGearsystemCore->KeyReleased(Joypad_1, key);
}

-(void)pause
{
    theGearsystemCore->Pause(true);
    audioEnabled = NO;
}

-(void)resume
{
    theGearsystemCore->Pause(false);
    audioEnabled = YES;
}

-(BOOL)paused
{
    return theGearsystemCore->IsPaused();
}

-(void)reset
{
    theGearsystemCore->SaveRam();
    theGearsystemCore->ResetROM();
    theGearsystemCore->LoadRam();
}

-(void)save
{
    theGearsystemCore->SaveRam();
}

- (void)setAudio: (BOOL)enabled
{
    audioEnabled = enabled;
}

- (void)resetAudio
{
    theSoundQueue->stop();
    theSoundQueue->start(44100, 2);
}

- (void)saveState
{
    saveStatePending = YES;
}

- (void)loadState
{
    loadStatePending = YES;
}

@end
