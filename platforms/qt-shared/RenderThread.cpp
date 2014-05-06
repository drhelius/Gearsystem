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

#include "RenderThread.h"
#include "GLFrame.h"
#include "Emulator.h"

RenderThread::RenderThread(GLFrame* pGLFrame) : QThread(), m_pGLFrame(pGLFrame)
{
    m_bPaused = false;
    m_bDoRendering = true;
    m_pFrameBuffer = new GS_Color[GS_SMS_WIDTH * GS_SMS_HEIGHT];
    m_iWidth = 0;
    m_iHeight = 0;
    InitPointer(m_pEmulator);
    m_bFiltering = false;
    m_bResizeEvent = false;
    m_GBTexture = 0;
}

RenderThread::~RenderThread()
{
}

void RenderThread::ResizeViewport(const QSize &size)
{
    m_iWidth = size.width();
    m_iHeight = size.height();
    m_bResizeEvent = true;
}

void RenderThread::Stop()
{
    m_bDoRendering = false;
}

void RenderThread::Pause()
{
    m_bPaused = true;
}

void RenderThread::Resume()
{
    m_bPaused = false;
}

bool RenderThread::IsRunningEmulator()
{
    return m_bDoRendering;
}

void RenderThread::SetEmulator(Emulator* pEmulator)
{
    m_pEmulator = pEmulator;
}

void RenderThread::run()
{
    m_pGLFrame->makeCurrent();
    
    Init();

    while (m_bDoRendering)
    {
        if (!m_bPaused)
        {
            m_pEmulator->RunToVBlank(m_pFrameBuffer);

            if (m_bResizeEvent)
            {
                m_bResizeEvent = false;
            }

            RenderFrame();

            m_pGLFrame->swapBuffers();
        }
    }
    
    SafeDeleteArray(m_pFrameBuffer);
    glDeleteTextures(1, &m_GBTexture);
}

void RenderThread::Init()
{
    for (int y = 0; y < GS_SMS_HEIGHT; ++y)
    {
        for (int x = 0; x < GS_SMS_WIDTH; ++x)
        {
            int pixel = (y * GS_SMS_WIDTH) + x;
            m_pFrameBuffer[pixel].red = m_pFrameBuffer[pixel].green =
                    m_pFrameBuffer[pixel].blue = 0x00;
            m_pFrameBuffer[pixel].alpha = 0xFF;
        }
    }

#ifndef __APPLE__
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        /* Problem: glewInit failed, something is seriously wrong. */
        Log("GLEW Error: %s\n", glewGetErrorString(err));
    }
    Log("Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
#endif

    glGenTextures(1, &m_GBTexture);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_GBTexture);
    SetupTexture((GLvoid*) m_pFrameBuffer);
}

void RenderThread::SetupTexture(GLvoid* data)
{
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GS_SMS_WIDTH, GS_SMS_HEIGHT, 0,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
}

void RenderThread::RenderFrame()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, m_GBTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, GS_SMS_WIDTH, GS_SMS_HEIGHT,
            GL_RGBA, GL_UNSIGNED_BYTE, (GLvoid*) m_pFrameBuffer);
    if (m_bFiltering)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
    RenderQuad(m_iWidth, m_iHeight, false);
}

void RenderThread::RenderQuad(int viewportWidth, int viewportHeight, bool mirrorY)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    if (mirrorY)
        gluOrtho2D(0, viewportWidth, 0, viewportHeight);
    else
        gluOrtho2D(0, viewportWidth, viewportHeight, 0);
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

void RenderThread::SetBilinearFiletering(bool enabled)
{
    m_bFiltering = enabled;
}

