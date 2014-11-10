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

#include "EmulatorInput.h"
#import "Emulator.h"

EmulatorInput::EmulatorInput(Emulator* pEmulator)
{
    m_pEmulator = pEmulator;
    m_pInputCallbackController = new InputCallback<EmulatorInput> (this, &EmulatorInput::InputController);
    m_pInputCallbackButtons = new InputCallback<EmulatorInput> (this, &EmulatorInput::InputButtons);
}

EmulatorInput::~EmulatorInput()
{
    SafeDelete(m_pInputCallbackController);
    SafeDelete(m_pInputCallbackButtons);
}

void EmulatorInput::Init()
{
    InputManager::Instance().ClearRegionEvents();
    for (int i = 0; i < 4; i++)
        m_bController[i] = false;
    
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
    {
        CGRect screenBounds = [[UIScreen mainScreen] bounds];
        switch ((int)screenBounds.size.height)
        {
            case 480:
            {
                // iPhone 4
                InputManager::Instance().AddCircleRegionEvent(221.0f, 330.0f, 34.0f, m_pInputCallbackButtons, 1, false);
                InputManager::Instance().AddCircleRegionEvent(280.0f, 330.0f, 34.0f, m_pInputCallbackButtons, 2, false);
                InputManager::Instance().AddCircleRegionEvent(163.0f, 271.0f, 25.0f, m_pInputCallbackButtons, 3, false);
                InputManager::Instance().AddCircleRegionEvent(69.0f, 315.0f, 50.0f, m_pInputCallbackController, 0, true);
                break;
            }
            case 568:
            {
                // iPhone 5
                InputManager::Instance().AddCircleRegionEvent(221.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 1, false);
                InputManager::Instance().AddCircleRegionEvent(280.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 2, false);
                InputManager::Instance().AddCircleRegionEvent(163.0f, 298.0f, 25.0f, m_pInputCallbackButtons, 3, false);
                InputManager::Instance().AddCircleRegionEvent(69.0f, 342.0f, 50.0f, m_pInputCallbackController, 0, true);
                break;
            }
            case 667:
            {
                // iPhone 6
                InputManager::Instance().AddCircleRegionEvent(221.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 1, false);
                InputManager::Instance().AddCircleRegionEvent(280.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 2, false);
                InputManager::Instance().AddCircleRegionEvent(163.0f, 298.0f, 25.0f, m_pInputCallbackButtons, 3, false);
                InputManager::Instance().AddCircleRegionEvent(69.0f, 342.0f, 50.0f, m_pInputCallbackController, 0, true);
                break;
            }
            case 736:
            {
                // iPhone 6 Plus
                InputManager::Instance().AddCircleRegionEvent(221.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 1, false);
                InputManager::Instance().AddCircleRegionEvent(280.0f, 358.0f, 34.0f, m_pInputCallbackButtons, 2, false);
                InputManager::Instance().AddCircleRegionEvent(163.0f, 298.0f, 25.0f, m_pInputCallbackButtons, 3, false);
                InputManager::Instance().AddCircleRegionEvent(69.0f, 342.0f, 50.0f, m_pInputCallbackController, 0, true);
                break;
            }
        }
    }
    else
    {
        InputManager::Instance().AddCircleRegionEvent(530.0f, 718.0f, 75.0f, m_pInputCallbackButtons, 1, false);
        InputManager::Instance().AddCircleRegionEvent(665.0f, 718.0f, 75.0f, m_pInputCallbackButtons, 2, false);
        InputManager::Instance().AddCircleRegionEvent(391.0f, 576.0f, 60.0f, m_pInputCallbackButtons, 3, false);
        InputManager::Instance().AddCircleRegionEvent(168.0f, 680.0f, 116.0f, m_pInputCallbackController, 0, true);
    }
}

void EmulatorInput::InputController(stInputCallbackParameter parameter, int id)
{
    bool bNewController[4];
    for (int i = 0; i < 4; i++)
        bNewController[i] = false;
    
    if (parameter.type != PRESS_END)
    {
        float length = parameter.vector.length();
        
        float minLength = 25.0f;
        
        if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone)
        {
            minLength = 11.0f;
        }
        
        if (length >= minLength)
        {
            float angle = atan2f(parameter.vector.x, -parameter.vector.y) * 57.29577951f;
            
            bNewController[0] = ((angle >= 35.0f) && (angle <= 145.0f));
            bNewController[1] = ((angle <= -35.0f) && (angle >= -145.0f));
            bNewController[2] = ((angle >= -55.0f) && (angle <= 55.0f));
            bNewController[3] = ((angle >= 125.0f) || (angle <= -125.0f));
        }
    }
    
    for (int i = 0; i < 4; i++)
    {
        if (bNewController[i] != m_bController[i])
        {
            m_bController[i] = bNewController[i];
            
            GS_Keys key = GS_Keys::Key_Start;
            
            switch (i)
            {
                case 0:
                    key = GS_Keys::Key_Right;
                    break;
                case 1:
                    key = GS_Keys::Key_Left;
                    break;
                case 2:
                    key = GS_Keys::Key_Up;
                    break;
                case 3:
                    key = GS_Keys::Key_Down;
                    break;
            }
            
            if (m_bController[i])
            {
                Log("dir press %d", i);
                [m_pEmulator keyPressed:key];
            }
            else
            {
                Log("dir release %d", i);
                [m_pEmulator keyReleased:key];
            }
        }
    }
}

void EmulatorInput::InputButtons(stInputCallbackParameter parameter, int id)
{
    GS_Keys key;
    
    switch (id) {
        case 1:
            key = GS_Keys::Key_1;
            break;
        case 2:
            key = GS_Keys::Key_2;
            break;
        case 3:
            key = GS_Keys::Key_Start;
            break;
        default:
            return;
    }
    
    if (parameter.type == PRESS_START)
    {
        Log("press %d", id);
        [m_pEmulator keyPressed:key];
    }
    else if (parameter.type == PRESS_END)
    {
        Log("release %d", id);
        [m_pEmulator keyReleased:key];
    }
}
