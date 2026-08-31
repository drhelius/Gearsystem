/*
 * Gearsystem - Sega Master System / Game Gear Emulator
 * Copyright (C) 2013 Ignacio Sanchez
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 */

#import "GearsystemEmulator.h"

#import <AVFAudio/AVFAudio.h>

#include <algorithm>
#include <string.h>
#include <strings.h>

#include "IOSAudioQueue.h"

#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#include "miniz.h"
#undef MINIZ_NO_ZLIB_COMPATIBLE_NAMES

#undef MIN
#undef MAX
#include "../../../src/gearsystem.h"

bool g_mcp_stdio_mode = false;

static NSString* const GearsystemEmulatorErrorDomain = @"me.ignaciosanchez.gearsystem.emulator";

static bool IsROMArchiveEntry(const char* filename)
{
    const char* extension = strrchr(filename, '.');
    if (!extension)
        return false;

    return (strcasecmp(extension, ".sms") == 0) ||
           (strcasecmp(extension, ".gg") == 0) ||
           (strcasecmp(extension, ".sg") == 0) ||
           (strcasecmp(extension, ".mv") == 0) ||
           (strcasecmp(extension, ".bin") == 0) ||
           (strcasecmp(extension, ".rom") == 0);
}

static NSString* ROMCRCInArchive(NSURL* url)
{
    if (!url.isFileURL)
        return nil;

    mz_zip_archive archive;
    memset(&archive, 0, sizeof(archive));
    if (!mz_zip_reader_init_file(&archive, url.fileSystemRepresentation, 0))
        return nil;

    NSString* result = nil;
    mz_uint fileCount = mz_zip_reader_get_num_files(&archive);
    for (mz_uint index = 0; index < fileCount; index++)
    {
        mz_zip_archive_file_stat fileStat;
        if (!mz_zip_reader_file_stat(&archive, index, &fileStat))
            break;
        if (!IsROMArchiveEntry(fileStat.m_filename))
            continue;

        size_t size = 0;
        void* data = mz_zip_reader_extract_to_heap(&archive, index, &size, 0);
        if (!data)
            break;

        mz_ulong checksum = mz_crc32(MZ_CRC32_INIT, (const unsigned char*)data, size);
        free(data);
        result = [NSString stringWithFormat:@"%08X", (unsigned int)checksum];
        break;
    }

    mz_zip_reader_end(&archive);
    return result;
}

static Cartridge::CartridgeSystem SystemForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Cartridge::CartridgeSMS;
        case 2:
            return Cartridge::CartridgeGG2ASIC;
        case 3:
            return Cartridge::CartridgeGG2ASICSMSMode;
        case 4:
            return Cartridge::CartridgeGG1ASIC;
        case 5:
            return Cartridge::CartridgeGG1ASICSMSMode;
        case 6:
            return Cartridge::CartridgeSG1000;
        case 7:
            return Cartridge::CartridgeSG1000II;
        default:
            return Cartridge::CartridgeUnknownSystem;
    }
}

static Cartridge::CartridgeZones ZoneForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Cartridge::CartridgeJapanSMS;
        case 2:
            return Cartridge::CartridgeExportSMS;
        case 3:
            return Cartridge::CartridgeJapanGG;
        case 4:
            return Cartridge::CartridgeExportGG;
        case 5:
            return Cartridge::CartridgeInternationalGG;
        default:
            return Cartridge::CartridgeUnknownZone;
    }
}

static Cartridge::CartridgeRegions TimingForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Cartridge::CartridgeNTSC;
        case 2:
            return Cartridge::CartridgePAL;
        default:
            return Cartridge::CartridgeUnknownRegion;
    }
}

static Cartridge::CartridgeTypes MapperForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Cartridge::CartridgeRomOnlyMapper;
        case 2:
            return Cartridge::CartridgeSegaMapper;
        case 3:
            return Cartridge::CartridgeCodemastersMapper;
        case 4:
            return Cartridge::CartridgeKoreanMapper;
        case 5:
            return Cartridge::CartridgeSG1000Mapper;
        case 6:
            return Cartridge::CartridgeMSXMapper;
        case 7:
            return Cartridge::CartridgeJanggunMapper;
        case 8:
            return Cartridge::CartridgeKorean2000XOR1FMapper;
        case 9:
            return Cartridge::CartridgeKoreanMSX32KB2000Mapper;
        case 10:
            return Cartridge::CartridgeKoreanMSXSMS8000Mapper;
        case 11:
            return Cartridge::CartridgeKoreanSMS32KB2000Mapper;
        case 12:
            return Cartridge::CartridgeKoreanMSX8KB0300Mapper;
        case 13:
            return Cartridge::CartridgeKorean0000XORFFMapper;
        case 14:
            return Cartridge::CartridgeKoreanFFFFHiComMapper;
        case 15:
            return Cartridge::CartridgeKoreanFFFEMapper;
        case 16:
            return Cartridge::CartridgeKoreanBFFCMapper;
        case 17:
            return Cartridge::CartridgeKoreanFFF3FFFCMapper;
        case 18:
            return Cartridge::CartridgeKoreanMDFFF5Mapper;
        case 19:
            return Cartridge::CartridgeKoreanMDFFF0Mapper;
        case 20:
            return Cartridge::CartridgeJumboDahjeeMapper;
        case 21:
            return Cartridge::CartridgeEeprom93C46Mapper;
        case 22:
            return Cartridge::CartridgeMulti4PAKAllActionMapper;
        case 23:
            return Cartridge::CartridgeIratahackMapper;
        default:
            return Cartridge::CartridgeNotSupported;
    }
}

static Video::Overscan OverscanForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Video::OverscanTopBottom;
        case 2:
            return Video::OverscanFull284;
        case 3:
            return Video::OverscanFull320;
        default:
            return Video::OverscanDisabled;
    }
}

static Video::HideLeftBar HideLeftBarForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return Video::HideLeftBarAuto;
        case 2:
            return Video::HideLeftBarAlways;
        default:
            return Video::HideLeftBarNo;
    }
}

static GearsystemCore::GlassesConfig GlassesForOption(NSInteger option)
{
    switch (option)
    {
        case 1:
            return GearsystemCore::GlassesLeftEye;
        case 2:
            return GearsystemCore::GlassesRightEye;
        default:
            return GearsystemCore::GlassesBothEyes;
    }
}

@interface GearsystemEmulator ()
{
    GearsystemCore* m_core;
    u16* m_frameBuffer;
    s16* m_audioBuffer;
    IOSAudioQueue m_audioQueue;
    uint32_t m_pressedButtons;
    BOOL m_loaded;
    BOOL m_muted;
    BOOL m_noSpriteLimit;
    NSInteger m_overscan;
    NSInteger m_hideLeftBar;
    NSInteger m_glasses;
    NSInteger m_ym2413;
    NSInteger m_psgVolume;
    NSInteger m_fmVolume;
    NSInteger m_saveStateSlot;
    NSInteger m_frameWidth;
    NSInteger m_frameHeight;
    double m_framesPerSecond;
    Cartridge::ForceConfiguration m_configuration;
    AVAudioEngine* m_audioEngine;
    AVAudioSourceNode* m_audioSourceNode;
}

- (void)applyConfiguration;
- (void)updateRuntimeInfo;
- (void)configureAudio;
- (void)audioEngineConfigurationChanged:(NSNotification*)notification;
- (void)clearAudio;
- (void)enqueueAudioSamples:(const s16*)samples count:(int)count;
- (OSStatus)renderAudioFrames:(AVAudioFrameCount)frameCount outputData:(AudioBufferList*)outputData silence:(BOOL*)isSilence;

@end

@implementation GearsystemEmulator

+ (NSString*)romCRCInArchiveAtURL:(NSURL*)url
{
    return ROMCRCInArchive(url);
}

- (instancetype)init
{
    self = [super init];

    if (self)
    {
        m_core = new GearsystemCore();
        m_core->Init(GS_PIXEL_RGB565);

        m_frameBuffer = new u16[GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN]();
        m_audioBuffer = new s16[GS_AUDIO_BUFFER_SIZE]();
        m_audioQueue.Configure(GS_AUDIO_QUEUE_SIZE, 3);
        m_pressedButtons = 0;
        m_loaded = NO;
        m_muted = NO;
        m_noSpriteLimit = NO;
        m_overscan = 0;
        m_hideLeftBar = 0;
        m_glasses = 0;
        m_ym2413 = 0;
        m_psgVolume = 100;
        m_fmVolume = 100;
        m_saveStateSlot = 1;
        m_frameWidth = GS_RESOLUTION_SMS_WIDTH;
        m_frameHeight = GS_RESOLUTION_SMS_HEIGHT;
        m_framesPerSecond = GS_FRAMES_PER_SECOND_NTSC;
        m_configuration.type = Cartridge::CartridgeNotSupported;
        m_configuration.zone = Cartridge::CartridgeUnknownZone;
        m_configuration.region = Cartridge::CartridgeUnknownRegion;
        m_configuration.system = Cartridge::CartridgeUnknownSystem;

        [self applyConfiguration];
        [self configureAudio];
    }

    return self;
}

- (void)dealloc
{
    [NSNotificationCenter.defaultCenter removeObserver:self];
    [self stopAudio];

    if (m_loaded)
    {
        m_core->SaveRam();
    }

    SafeDeleteArray(m_audioBuffer);
    SafeDeleteArray(m_frameBuffer);
    SafeDelete(m_core);
}

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
{
    m_configuration.system = SystemForOption(system);
    m_configuration.zone = ZoneForOption(region);
    m_configuration.type = MapperForOption(mapper);
    m_configuration.region = TimingForOption(timing);
    m_overscan = overscan;
    m_hideLeftBar = hideLeftBar;
    m_noSpriteLimit = noSpriteLimit;
    m_glasses = glasses;
    m_ym2413 = ym2413;
    m_psgVolume = std::min(std::max(psgVolume, (NSInteger)0), (NSInteger)200);
    m_fmVolume = std::min(std::max(fmVolume, (NSInteger)0), (NSInteger)200);

    if (saveStateSlot < 1)
    {
        m_saveStateSlot = 1;
    }
    else if (saveStateSlot > 5)
    {
        m_saveStateSlot = 5;
    }
    else
    {
        m_saveStateSlot = saveStateSlot;
    }

    [self applyConfiguration];
}

- (void)applyConfiguration
{
    m_core->GetVideo()->SetOverscan(OverscanForOption(m_overscan));
    m_core->GetVideo()->SetHideLeftBar(HideLeftBarForOption(m_hideLeftBar));
    m_core->GetVideo()->SetNoSpriteLimit(m_noSpriteLimit);
    m_core->SetGlassesConfig(GlassesForOption(m_glasses));
    m_core->GetAudio()->DisableYM2413(m_ym2413 == 1);
    m_core->GetAudio()->SetPSGVolume((float)m_psgVolume / 100.0f);
    m_core->GetAudio()->SetFMVolume((float)m_fmVolume / 100.0f);
}

- (void)updateRuntimeInfo
{
    GS_RuntimeInfo runtimeInfo;

    if (m_core->GetRuntimeInfo(runtimeInfo))
    {
        m_frameWidth = runtimeInfo.screen_width;
        m_frameHeight = runtimeInfo.screen_height;
        m_framesPerSecond = runtimeInfo.fps;
    }
}

- (BOOL)loadROMAtURL:(NSURL*)url error:(NSError**)error
{
    if (!url.isFileURL)
    {
        if (error)
        {
            *error = [NSError errorWithDomain:GearsystemEmulatorErrorDomain
                                         code:1
                                     userInfo:@{NSLocalizedDescriptionKey: @"The selected item is not a local ROM file."}];
        }

        return NO;
    }

    if (m_loaded)
    {
        m_core->SaveRam();
    }

    [self releaseAllButtons];
    [self clearAudio];

    [self applyConfiguration];
    BOOL loaded = m_core->LoadROM(url.fileSystemRepresentation, &m_configuration);

    if (!loaded)
    {
        m_loaded = NO;

        if (error)
        {
            *error = [NSError errorWithDomain:GearsystemEmulatorErrorDomain
                                         code:2
                                     userInfo:@{NSLocalizedDescriptionKey: @"Gearsystem could not load this ROM."}];
        }

        return NO;
    }

    m_core->LoadRam();
    m_core->Pause(false);
    m_loaded = YES;
    memset(m_frameBuffer, 0,
        GS_RESOLUTION_MAX_WIDTH_WITH_OVERSCAN * GS_RESOLUTION_MAX_HEIGHT_WITH_OVERSCAN * sizeof(u16));
    [self updateRuntimeInfo];

    return YES;
}

- (void)runFrame
{
    if (!m_loaded || m_core->IsPaused())
    {
        return;
    }

    int sampleCount = 0;
    m_core->RunToVBlank(reinterpret_cast<u8*>(m_frameBuffer), m_audioBuffer, &sampleCount);
    [self updateRuntimeInfo];

    if (!m_muted && (sampleCount > 0))
    {
        [self enqueueAudioSamples:m_audioBuffer count:sampleCount];
    }
}

- (void)setButton:(GearsystemButton)button pressed:(BOOL)pressed
{
    if (!m_loaded)
    {
        return;
    }

    uint32_t buttonMask = 1U << (uint32_t)button;
    BOOL wasPressed = (m_pressedButtons & buttonMask) != 0;

    if (pressed == wasPressed)
    {
        return;
    }

    GS_Keys key;

    switch (button)
    {
        case GearsystemButtonUp:
            key = Key_Up;
            break;
        case GearsystemButtonDown:
            key = Key_Down;
            break;
        case GearsystemButtonLeft:
            key = Key_Left;
            break;
        case GearsystemButtonRight:
            key = Key_Right;
            break;
        case GearsystemButtonOne:
            key = Key_1;
            break;
        case GearsystemButtonTwo:
            key = Key_2;
            break;
        case GearsystemButtonStart:
            key = Key_Start;
            break;
    }

    if (pressed)
    {
        m_pressedButtons |= buttonMask;
        m_core->KeyPressed(Joypad_1, key);
    }
    else
    {
        m_pressedButtons &= ~buttonMask;
        m_core->KeyReleased(Joypad_1, key);
    }
}

- (void)releaseAllButtons
{
    if (!m_core)
    {
        return;
    }

    static const GearsystemButton buttons[] =
    {
        GearsystemButtonUp,
        GearsystemButtonDown,
        GearsystemButtonLeft,
        GearsystemButtonRight,
        GearsystemButtonOne,
        GearsystemButtonTwo,
        GearsystemButtonStart
    };

    for (GearsystemButton button : buttons)
    {
        if ((m_pressedButtons & (1U << (uint32_t)button)) != 0)
        {
            [self setButton:button pressed:NO];
        }
    }
}

- (void)pause
{
    if (m_loaded)
    {
        [self releaseAllButtons];
        m_core->Pause(true);
    }
}

- (void)resume
{
    if (m_loaded)
    {
        m_core->Pause(false);
    }
}

- (void)reset
{
    if (!m_loaded)
    {
        return;
    }

    [self releaseAllButtons];
    m_core->SaveRam();
    [self applyConfiguration];
    m_core->ResetROMPreservingRAM(&m_configuration);
    [self updateRuntimeInfo];
    [self clearAudio];
}

- (void)saveRAM
{
    if (m_loaded)
    {
        m_core->SaveRam();
    }
}

- (void)saveState
{
    if (m_loaded)
    {
        m_core->SaveState(NULL, (int)m_saveStateSlot);
    }
}

- (void)loadState
{
    if (m_loaded)
    {
        [self releaseAllButtons];
        m_core->LoadState(NULL, (int)m_saveStateSlot);
        [self clearAudio];
    }
}

- (BOOL)isLoaded
{
    return m_loaded;
}

- (BOOL)isPaused
{
    return !m_loaded || m_core->IsPaused();
}

- (BOOL)isMuted
{
    return m_muted;
}

- (void)setMuted:(BOOL)muted
{
    m_muted = muted;

    if (muted)
    {
        [self clearAudio];
    }
}

- (const uint16_t*)frameBuffer
{
    return m_frameBuffer;
}

- (NSInteger)frameWidth
{
    return m_frameWidth;
}

- (NSInteger)frameHeight
{
    return m_frameHeight;
}

- (double)framesPerSecond
{
    return m_framesPerSecond;
}

- (void)configureAudio
{
    m_audioEngine = [[AVAudioEngine alloc] init];
    AVAudioFormat* format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:GS_AUDIO_SAMPLE_RATE channels:2];
    __weak GearsystemEmulator* weakSelf = self;

    m_audioSourceNode = [[AVAudioSourceNode alloc] initWithFormat:format
                                                     renderBlock:^OSStatus(BOOL* isSilence,
                                                                         const AudioTimeStamp* timestamp,
                                                                         AVAudioFrameCount frameCount,
                                                                         AudioBufferList* outputData)
    {
        UNUSED(timestamp);
        GearsystemEmulator* strongSelf = weakSelf;

        if (!strongSelf)
        {
            *isSilence = YES;

            for (UInt32 bufferIndex = 0; bufferIndex < outputData->mNumberBuffers; ++bufferIndex)
            {
                AudioBuffer* buffer = &outputData->mBuffers[bufferIndex];
                memset(buffer->mData, 0, buffer->mDataByteSize);
            }

            return noErr;
        }

        return [strongSelf renderAudioFrames:frameCount outputData:outputData silence:isSilence];
    }];

    [m_audioEngine attachNode:m_audioSourceNode];
    [m_audioEngine connect:m_audioSourceNode to:m_audioEngine.mainMixerNode format:format];
    [m_audioEngine prepare];

    [NSNotificationCenter.defaultCenter addObserver:self
                                           selector:@selector(audioEngineConfigurationChanged:)
                                               name:AVAudioEngineConfigurationChangeNotification
                                             object:m_audioEngine];
}

- (void)startAudio
{
    if (m_audioEngine.isRunning)
    {
        return;
    }

    AVAudioSession* session = AVAudioSession.sharedInstance;
    NSError* error = nil;
    [session setCategory:AVAudioSessionCategoryAmbient
                    mode:AVAudioSessionModeDefault
                 options:AVAudioSessionCategoryOptionMixWithOthers
                   error:&error];

    if (!error)
    {
        NSError* preferenceError = nil;
        [session setPreferredSampleRate:GS_AUDIO_SAMPLE_RATE error:&preferenceError];
        preferenceError = nil;
        [session setPreferredIOBufferDuration:512.0 / GS_AUDIO_SAMPLE_RATE error:&preferenceError];
        [session setActive:YES error:&error];
    }

    [self clearAudio];

    if (!error)
    {
        [m_audioEngine startAndReturnError:&error];
    }

    if (error)
    {
        NSLog(@"Unable to start Gearsystem audio: %@", error.localizedDescription);
    }
}

- (void)stopAudio
{
    if (m_audioEngine.isRunning)
    {
        [m_audioEngine pause];
    }

    [self clearAudio];

    NSError* error = nil;
    [AVAudioSession.sharedInstance setActive:NO
                                 withOptions:AVAudioSessionSetActiveOptionNotifyOthersOnDeactivation
                                       error:&error];

    if (error)
    {
        NSLog(@"Unable to stop Gearsystem audio: %@", error.localizedDescription);
    }
}

- (void)audioEngineConfigurationChanged:(NSNotification*)notification
{
    UNUSED(notification);
    [self clearAudio];

    __weak GearsystemEmulator* weakSelf = self;
    dispatch_async(dispatch_get_main_queue(), ^{
        GearsystemEmulator* strongSelf = weakSelf;

        if (!strongSelf || !strongSelf->m_loaded ||
            strongSelf->m_core->IsPaused() || strongSelf->m_audioEngine.isRunning)
        {
            return;
        }

        [strongSelf->m_audioEngine prepare];

        NSError* error = nil;
        [strongSelf->m_audioEngine startAndReturnError:&error];

        if (error)
        {
            NSLog(@"Unable to restart Gearsystem audio: %@", error.localizedDescription);
        }
    });
}

- (void)clearAudio
{
    m_audioQueue.Reset();
}

- (void)enqueueAudioSamples:(const s16*)samples count:(int)count
{
    if (count > 0)
        m_audioQueue.Write(samples, (uint32_t)count);
}

- (OSStatus)renderAudioFrames:(AVAudioFrameCount)frameCount outputData:(AudioBufferList*)outputData silence:(BOOL*)isSilence
{
    bool audible = false;

    if (outputData->mNumberBuffers >= 2)
    {
        float* left = (float*)outputData->mBuffers[0].mData;
        float* right = (float*)outputData->mBuffers[1].mData;
        audible = m_audioQueue.Render(left, right, (uint32_t)frameCount);
    }
    else if (outputData->mNumberBuffers == 1)
    {
        float* output = (float*)outputData->mBuffers[0].mData;
        audible = m_audioQueue.RenderInterleaved(output, (uint32_t)frameCount);
    }

    *isSilence = !audible;
    return noErr;
}

@end
