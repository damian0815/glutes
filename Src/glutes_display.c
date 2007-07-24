/*
 * glutes_display.c
 *
 * Display message posting, context buffer swapping.
 *
 * Copyright (c) 2005 Joachim Pouderoux. All Rights Reserved.
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Fri Dec 3 1999
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Marks the current window to have the redisplay performed when possible...
 */
void FGAPIENTRY glutPostRedisplay( void )
{
    glutes_assert_ready;
    glutes_assert_window;
    fgStructure.Window->State.Redisplay = GL_TRUE;
}

/*
 * Swaps the buffers for the current window (if any)
 */
void FGAPIENTRY glutSwapBuffers( void )
{
    glutes_assert_ready;
    glutes_assert_window;

    glFlush( );
    /*if( ! fgStructure.Window->Window.DoubleBuffered )
        return;*/
#if 0
#if TARGET_HOST_UNIX_X11
    glXSwapBuffers( fgDisplay.Display, fgStructure.Window->Window.Handle );
#elif TARGET_HOST_WIN32
    SwapBuffers( fgStructure.Window->Window.Device );
#endif
#endif

	if(fgStructure.Window->Window.SurfaceType == EGL_WINDOW_BIT)
	{
		eglSwapBuffers(fgDisplay.eglDisplay, fgStructure.Window->Window.Surface);
	}
#if TARGET_HOST_WIN32
	else if(fgStructure.Window->Window.SurfaceType == EGL_PIXMAP_BIT)
	{
		InvalidateRect(fgStructure.Window->Window.Handle, NULL, FALSE);
	}
#endif

    /* GLUT_FPS env var support */
    if( fgState.FPSInterval )
    {
        GLint t = glutGet( GLUT_ELAPSED_TIME );
        fgState.SwapCount++;
        if( fgState.SwapTime == 0 )
            fgState.SwapTime = t;
        else if( t - fgState.SwapTime > fgState.FPSInterval )
        {
            float time = 0.001f * ( t - fgState.SwapTime );
            float fps = ( float )fgState.SwapCount / time;
            fprintf( stderr,
                     "freeglut: %d frames in %.2f seconds = %.2f FPS\n",
                     fgState.SwapCount, time, fps );
            fgState.SwapTime = t;
            fgState.SwapCount = 0;
        }
    }
}

/*
 * Mark appropriate window to be displayed
 */
void FGAPIENTRY glutPostWindowRedisplay( int windowID )
{
    SFG_Window* window;

    glutes_assert_ready;
    window = fgWindowByID( windowID );
    glutes_return_if_fail( window );
    window->State.Redisplay = GL_TRUE;
}

/*** END OF FILE ***/
