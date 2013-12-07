# This file is generated automatically. Do not edit.
# Use project properties -> Build -> Qt -> Expert -> Custom Definitions.
TEMPLATE = app
DESTDIR = dist/Debug/CLang-MacOSX
TARGET = Gearsystem
VERSION = 1.0.0
CONFIG -= debug_and_release app_bundle lib_bundle
CONFIG += debug 
PKGCONFIG +=
QT = core gui opengl
SOURCES += ../../../src/Audio.cpp ../../../src/Cartridge.cpp ../../../src/CodemastersMemoryRule.cpp ../../../src/GameGearIOPorts.cpp ../../../src/GearsystemCore.cpp ../../../src/Input.cpp ../../../src/Memory.cpp ../../../src/MemoryRule.cpp ../../../src/Processor.cpp ../../../src/RomOnlyMemoryRule.cpp ../../../src/SegaMemoryRule.cpp ../../../src/SmsIOPorts.cpp ../../../src/Video.cpp ../../../src/audio/Blip_Buffer.cpp ../../../src/audio/Effects_Buffer.cpp ../../../src/audio/Multi_Buffer.cpp ../../../src/audio/Sms_Apu.cpp ../../../src/audio/Sound_Queue.cpp ../../../src/opcodes.cpp ../../../src/opcodes_cb.cpp ../../../src/opcodes_ed.cpp ../../qt-shared/About.cpp ../../qt-shared/Emulator.cpp ../../qt-shared/GLFrame.cpp ../../qt-shared/InputSettings.cpp ../../qt-shared/MainWindow.cpp ../../qt-shared/RenderThread.cpp ../../qt-shared/SoundSettings.cpp ../../qt-shared/VideoSettings.cpp ../../qt-shared/main.cpp
HEADERS += ../../../src/Audio.h ../../../src/Cartridge.h ../../../src/CodemastersMemoryRule.h ../../../src/EightBitRegister.h ../../../src/GameGearIOPorts.h ../../../src/GearsystemCore.h ../../../src/IOPorts.h ../../../src/Input.h ../../../src/Memory.h ../../../src/MemoryRule.h ../../../src/Memory_inline.h ../../../src/Processor.h ../../../src/Processor_inline.h ../../../src/RomOnlyMemoryRule.h ../../../src/SegaMemoryRule.h ../../../src/SixteenBitRegister.h ../../../src/SmsIOPorts.h ../../../src/Video.h ../../../src/audio/Blip_Buffer.h ../../../src/audio/Blip_Synth.h ../../../src/audio/Effects_Buffer.h ../../../src/audio/Multi_Buffer.h ../../../src/audio/Sms_Apu.h ../../../src/audio/Sms_Oscs.h ../../../src/audio/Sound_Queue.h ../../../src/audio/blargg_common.h ../../../src/audio/blargg_config.h ../../../src/audio/blargg_source.h ../../../src/definitions.h ../../../src/game_db.h ../../../src/gearsystem.h ../../../src/opcode_daa.h ../../../src/opcode_names.h ../../../src/opcode_timing.h ../../../src/opcodecb_names.h ../../../src/opcodedd_names.h ../../../src/opcodeddcb_names.h ../../../src/opcodeed_names.h ../../../src/opcodefd_names.h ../../../src/opcodefdcb_names.h ../../../src/opcodexx_names.h ../../qt-shared/About.h ../../qt-shared/Emulator.h ../../qt-shared/GLFrame.h ../../qt-shared/InputSettings.h ../../qt-shared/MainWindow.h ../../qt-shared/RenderThread.h ../../qt-shared/SoundSettings.h ../../qt-shared/VideoSettings.h
FORMS += ../../qt-shared/About.ui ../../qt-shared/InputSettings.ui ../../qt-shared/MainWindow.ui ../../qt-shared/SoundSettings.ui ../../qt-shared/VideoSettings.ui
RESOURCES +=
TRANSLATIONS +=
OBJECTS_DIR = build/Debug/CLang-MacOSX
MOC_DIR = 
RCC_DIR = 
UI_DIR = 
QMAKE_CC = clang
QMAKE_CXX = clang++
DEFINES += 
INCLUDEPATH += 
LIBS += -lSDL2main -lSDL2  
