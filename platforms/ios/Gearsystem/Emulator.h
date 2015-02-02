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

#import <Foundation/Foundation.h>

#ifdef __APPLE__
#if TARGET_IPHONE_SIMULATOR == 1 || TARGET_OS_IPHONE == 1
#define SDL_MAIN_HANDLED
#endif
#endif

#import "../../../src/gearsystem.h"
#import "EmulatorInput.h"
#include "texturemanager.h"

@interface Emulator : NSObject
{
    GearsystemCore* theGearsystemCore;
    GS_Color* theFrameBuffer;
    GS_Color* theTexture;
    EmulatorInput* theInput;
    GLuint GBTexture;
    Texture* scanlineTexture;
}

@property (nonatomic) float multiplier;
@property (nonatomic) BOOL retina;
@property (nonatomic) BOOL iPad;
@property (nonatomic) int scanlines;


- (void)update;
- (void)draw;
- (void)loadRomWithPath: (NSString *)filePath;
- (void)keyPressed: (GS_Keys)key;
- (void)keyReleased: (GS_Keys)key;
- (void)pause;
- (void)resume;
- (BOOL)paused;
- (void)reset;
- (void)save;
- (void)initGL;
- (void)shutdownGL;
- (void)renderFrame;
- (void)setupTextureWithData: (GLvoid*) data;
- (void)renderQuadWithViewportWidth: (float)viewportWidth andHeight: (float)viewportHeight andMirrorY: (BOOL)mirrorY;
- (void)setAudio: (BOOL)enabled;
- (void)resetAudio;

@end
