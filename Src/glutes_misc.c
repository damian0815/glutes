/*
 * glutes_misc.c
 *
 * Functions that didn't fit anywhere else...
 *
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 9 1999
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
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"

/*
 * TODO BEFORE THE STABLE RELEASE:
 *
 *  glutSetColor()     --
 *  glutGetColor()     --
 *  glutCopyColormap() --
 *  glutSetKeyRepeat() -- this is evil and should be removed from API
 */

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * This functions checks if an OpenGL extension is supported or not
 *
 * XXX Wouldn't this be simpler and clearer if we used strtok()?
 */
int FGAPIENTRY glutExtensionSupported( const char* extension )
{
  const char *extensions, *start;
  const int len = strlen( extension );

  /*
   * Make sure there is a current window, and thus a current context available
   */
  glutes_assert_ready;
  glutes_return_val_if_fail( fgStructure.Window != NULL, 0 );

  if (strchr(extension, ' '))
    return 0;
  start = extensions = (const char *) glGetString(GL_EXTENSIONS);

  /* XXX consider printing a warning to stderr that there's no current
   * rendering context.
   */
  glutes_return_val_if_fail( extensions != NULL, 0 );

  while (1) {
     const char *p = strstr(extensions, extension);
     if (!p)
        return 0;  /* not found */
     /* check that the match isn't a super string */
     if ((p == start || p[-1] == ' ') && (p[len] == ' ' || p[len] == 0))
        return 1;
     /* skip the false match and continue */
     extensions = p + len;
  }

  return 0 ;
}

/*
 * This function reports all the OpenGL errors that happened till now
 */
void FGAPIENTRY glutReportErrors( void )
{
    GLenum error;
    while( ( error = glGetError() ) != GL_NO_ERROR )
        fgWarning( "GL error: %d", error); //gluErrorString( error ) );
}

/*
 * Turns the ignore key auto repeat feature on and off
 *
 * DEPRECATED 11/4/02 - Do not use
 */
void FGAPIENTRY glutIgnoreKeyRepeat( int ignore )
{
    fgState.IgnoreKeyRepeat = ignore ? GL_TRUE : GL_FALSE;
}

/*
 * Hints the window system whether to generate key auto repeat, or not.
 * This is evil.
 *
 * XXX Is this also deprecated as of 20021104?
 */
void FGAPIENTRY glutSetKeyRepeat( int repeatMode )
{
#if TARGET_HOST_UNIX_X11

    glutes_assert_ready;

    switch( repeatMode )
    {
    case GLUT_KEY_REPEAT_OFF:   XAutoRepeatOff( fgDisplay.Display ); break;
    case GLUT_KEY_REPEAT_ON:    XAutoRepeatOn( fgDisplay.Display );  break;
    case GLUT_KEY_REPEAT_DEFAULT:
        {
            XKeyboardState keyboardState;

            XGetKeyboardControl( fgDisplay.Display, &keyboardState );
            glutSetKeyRepeat(
                keyboardState.global_auto_repeat == AutoRepeatModeOn ?
                GLUT_KEY_REPEAT_ON : GLUT_KEY_REPEAT_OFF
            );
        }
        break;

    default:
        fgError ("Invalid glutSetKeyRepeat mode: %d", repeatMode);
        break;
    }

#endif
}

/*
 * Forces the joystick callback to be executed
 */
void FGAPIENTRY glutForceJoystickFunc( void )
{
#ifndef _WIN32_WCE
    glutes_assert_ready;
    glutes_return_if_fail( fgStructure.Window != NULL );
    glutes_return_if_fail( FETCH_WCB( *( fgStructure.Window ), Joystick ) );
    fgJoystickPollWindow( fgStructure.Window );
#endif
}

/*
 *
 */
void FGAPIENTRY glutSetColor( int nColor, GLfloat red, GLfloat green, GLfloat blue )
{
    /*
     *
     */
}

/*
 *
 */
GLfloat FGAPIENTRY glutGetColor( int color, int component )
{
    /*
     *
     */
    return( 0.0f );
}

/*
 *
 */
void FGAPIENTRY glutCopyColormap( int window )
{
    /*
     *
     */
}

/*
 * Simulate a button click.
 */
void FGAPIENTRY glutSimulateButton(int button, int x, int y)
{
	glutes_assert_ready;
    glutes_return_if_fail( fgStructure.Window != NULL );
#if TARGET_HOST_WIN32
	if(button == GLUT_LEFT_BUTTON)
		button = WM_LBUTTONDOWN;
	else if(button == GLUT_MIDDLE_BUTTON)
		button = WM_MBUTTONDOWN;
	else if(button == GLUT_RIGHT_BUTTON)
		button = WM_RBUTTONDOWN;
	SendMessage(fgStructure.Window->Window.Handle, button, 0, MAKELPARAM(x, y));
#endif
}

/*** END OF FILE ***/
