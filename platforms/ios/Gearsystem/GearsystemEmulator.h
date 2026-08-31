/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013 Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, GearsystemButton)
{
    GearsystemButtonUp,
    GearsystemButtonDown,
    GearsystemButtonLeft,
    GearsystemButtonRight,
    GearsystemButtonOne,
    GearsystemButtonTwo,
    GearsystemButtonStart
};

@interface GearsystemEmulator : NSObject

@property (nonatomic, readonly, getter=isLoaded) BOOL loaded;
@property (nonatomic, readonly, getter=isPaused) BOOL paused;
@property (nonatomic, getter=isMuted) BOOL muted;
@property (nonatomic, readonly) const uint16_t* frameBuffer;
@property (nonatomic, readonly) NSInteger frameWidth;
@property (nonatomic, readonly) NSInteger frameHeight;
@property (nonatomic, readonly) double framesPerSecond;

+ (nullable NSString*)romCRCInArchiveAtURL:(NSURL*)url NS_SWIFT_NAME(romCRC(inArchiveAt:));
- (void)configureWithSystem:(NSInteger)system
                     region:(NSInteger)region
                     mapper:(NSInteger)mapper
                     timing:(NSInteger)timing
                   overscan:(NSInteger)overscan
                hideLeftBar:(NSInteger)hideLeftBar
              noSpriteLimit:(BOOL)noSpriteLimit
                    glasses:(NSInteger)glasses
                     ym2413:(NSInteger)ym2413
                   psgVolume:(NSInteger)psgVolume
                    fmVolume:(NSInteger)fmVolume
              saveStateSlot:(NSInteger)saveStateSlot
    NS_SWIFT_NAME(configure(system:region:mapper:timing:overscan:hideLeftBar:noSpriteLimit:glasses:ym2413:psgVolume:fmVolume:saveStateSlot:));
- (BOOL)loadROMAtURL:(NSURL*)url error:(NSError* _Nullable* _Nullable)error NS_SWIFT_NAME(loadROM(at:));
- (void)runFrame;
- (void)setButton:(GearsystemButton)button pressed:(BOOL)pressed;
- (void)releaseAllButtons;
- (void)pause;
- (void)resume;
- (void)reset;
- (void)saveRAM;
- (void)saveState;
- (void)loadState;
- (void)startAudio;
- (void)stopAudio;

@end

NS_ASSUME_NONNULL_END
