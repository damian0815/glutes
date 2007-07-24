/*
 * glutes_init.c
 *
 * Various freeglut initialization functions.
 *
 * Copyright (c) 2005 Joachim Pouderoux. All Rights Reserved.
 * Copyright (c) 1999-2000 Pawel W. Olszta. All Rights Reserved.
 * Written by Pawel W. Olszta, <olszta@sourceforge.net>
 * Creation date: Thu Dec 2 1999
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
 *  fgDeinitialize()        -- Win32's OK, X11 needs the OS-specific
 *                             deinitialization done
 *  glutInitDisplayString() -- display mode string parsing
 *
 * Wouldn't it be cool to use gettext() for error messages? I just love
 * bash saying  "nie znaleziono pliku" instead of "file not found" :)
 * Is gettext easily portable?
 */

/* -- GLOBAL VARIABLES ----------------------------------------------------- */

/*
 * A structure pointed by g_pDisplay holds all information
 * regarding the display, screen, root window etc.
 */
SFG_Display fgDisplay;

/*
 * The settings for the current freeglut session
 */
SFG_State fgState = { { -1, -1, GL_FALSE },  /* Position */
                      { 300, 300, GL_TRUE }, /* Size */
                      GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH,  /* DisplayMode */
                      GL_FALSE,              /* Initialised */
                      GL_FALSE,              /* ForceDirectContext */
                      GL_TRUE,               /* TryDirectContext */
                      GL_FALSE,              /* ForceIconic */
                      GL_FALSE,              /* UseCurrentContext */
                      GL_FALSE,              /* GLDebugSwitch */
                      GL_FALSE,              /* XSyncSwitch */
                      GL_TRUE,               /* IgnoreKeyRepeat */
                      0xffffffff,            /* Modifiers */
                      0,                     /* FPSInterval */
                      0,                     /* SwapCount */
                      0,                     /* SwapTime */
#if TARGET_HOST_WIN32
                      { 0, GL_FALSE },       /* Time */
#else
                      { { 0, 0 }, GL_FALSE },
#endif
                      { NULL, NULL },         /* Timers */
                      { NULL, NULL },         /* FreeTimers */
                      NULL,                   /* IdleCallback */
                      0,                      /* ActiveMenus */
                      NULL,                   /* MenuStateCallback */
                      NULL,                   /* MenuStatusCallback */
                      { 640, 480, GL_TRUE },  /* GameModeSize */
                      16,                     /* GameModeDepth */
                      72,                     /* GameModeRefresh */
                      GLUT_ACTION_EXIT,       /* ActionOnWindowClose */
                      GLUT_EXEC_STATE_INIT,   /* ExecState */
                      NULL                    /* ProgramName */
};


/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */


#define GETRAWFRAMEBUFFER   0x00020001

void __glInit()
{
	EGLint majorVersion;
	EGLint minorVersion;
//	EGLConfig cfg;
	/* EGL Setup */
#ifdef _WIN32_WCE
	HDC hdc = GetDC(NULL);
	ExtEscape(hdc, GETRAWFRAMEBUFFER, 0, NULL, sizeof(RawFrameBufferInfo), (char *) &fgDisplay.FrameBufferInfo);
	ReleaseDC(NULL, hdc);
#endif
	
	if(fgDisplay.eglDisplay)
		return;

#if TARGET_HOST_UNIX_X11
	fgDisplay.eglDisplay = eglGetDisplay((NativeDisplayType)fgDisplay.Display);
#else
	fgDisplay.eglDisplay = eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
#endif

	if(!eglInitialize(fgDisplay.eglDisplay, &majorVersion, &minorVersion))
		fgError("Unable to initialize OpenGL|ES!");
	
}

void __glDestroy()
{
	if(!fgDisplay.eglDisplay)
		return;
	eglMakeCurrent(fgDisplay.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
	eglTerminate(fgDisplay.eglDisplay);
	fgDisplay.eglDisplay = 0;
}

/*
 * A call to this function should initialize all the display stuff...
 */
void fgInitialize( const char* displayName )
{
#if TARGET_HOST_UNIX_X11
    fgDisplay.Display = XOpenDisplay( displayName );

    if( fgDisplay.Display == NULL )
        fgError( "failed to open display '%s'", XDisplayName( displayName ) );

#if !defined(USE_EGL)
    if( !glXQueryExtension( fgDisplay.Display, NULL, NULL ) )
        fgError( "OpenGL GLX extension not supported by display '%s'",
            XDisplayName( displayName ) );
#endif

    fgDisplay.Screen = DefaultScreen( fgDisplay.Display );
    fgDisplay.RootWindow = RootWindow(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidth  = DisplayWidth(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeight = DisplayHeight(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.ScreenWidthMM = DisplayWidthMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );
    fgDisplay.ScreenHeightMM = DisplayHeightMM(
        fgDisplay.Display,
        fgDisplay.Screen
    );

    fgDisplay.Connection = ConnectionNumber( fgDisplay.Display );

    /*
     * Create the window deletion atom
     */
    fgDisplay.DeleteWindow = XInternAtom(
        fgDisplay.Display,
        "WM_DELETE_WINDOW",
        FALSE
    );

#elif TARGET_HOST_WIN32

    WNDCLASS wc;
    ATOM atom;

    /*
     * What we need to do is to initialize the fgDisplay global structure here.
     */
    fgDisplay.Instance = GetModuleHandle( NULL );

    atom = GetClassInfo( fgDisplay.Instance, TEXT("GLUT|ES"), &wc );
    if( atom == 0 )
    {
        ZeroMemory( &wc, sizeof(WNDCLASS) );

        /*
         * Each of the windows should have its own device context, and we
         * want redraw events during Vertical and Horizontal Resizes by
         * the user.
         *
         * XXX Old code had "| CS_DBCLCKS" commented out.  Plans for the
         * XXX future?  Dead-end idea?
         */
#ifdef _WIN32_WCE
        wc.style          = 0;
#else
		wc.style		  = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
#endif
        wc.lpfnWndProc    = fgWindowProc;
        wc.cbClsExtra     = 0;
        wc.cbWndExtra     = 0;
        wc.hInstance      = fgDisplay.Instance;
#ifndef _WIN32_WCE
        wc.hIcon          = LoadIcon( fgDisplay.Instance, "GLUT_ICON" );
        if (!wc.hIcon)
          wc.hIcon        = LoadIcon( NULL, IDI_WINLOGO );
#endif
        wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
        wc.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH);;
        wc.lpszMenuName   = NULL;
        wc.lpszClassName  = TEXT("GLUT|ES");

        /*
         * Register the window class
         */
        atom = RegisterClass( &wc );
        assert( atom );
    }

    /*
     * The screen dimensions can be obtained via GetSystemMetrics() calls
     */
    fgDisplay.ScreenWidth  = GetSystemMetrics( SM_CXSCREEN );
    fgDisplay.ScreenHeight = GetSystemMetrics( SM_CYSCREEN );

    {
        HWND desktop = GetDesktopWindow( );
        HDC  context = GetDC( desktop );

        fgDisplay.ScreenWidthMM  = GetDeviceCaps( context, HORZSIZE );
        fgDisplay.ScreenHeightMM = GetDeviceCaps( context, VERTSIZE );

        ReleaseDC( desktop, context );
    }

#endif

	__glInit();
#ifndef _WIN32_WCE
    fgJoystickInit( 0 );
#endif

    fgState.Initialised = GL_TRUE;
}

/*
 * Perform the freeglut deinitialization...
 */
void fgDeinitialize( void )
{
    SFG_Timer *timer;

    if( !fgState.Initialised )
    {
        fgWarning( "fgDeinitialize(): "
                   "no valid initialization has been performed" );
        return;
    }

    /* fgState.Initialised = GL_FALSE; */

    /*
     * If there was a menu created, destroy the rendering context
     */
    if( fgStructure.MenuContext )
    {
        free( fgStructure.MenuContext );
        fgStructure.MenuContext = NULL;
    }

    fgDestroyStructure( );

    while( (timer = fgState.Timers.First) )
    {
        fgListRemove( &fgState.Timers, &timer->Node );
        free( timer );
    }

    while( (timer = fgState.FreeTimers.First) )
    {
        fgListRemove( &fgState.FreeTimers, &timer->Node );
        free( timer );
    }

#ifndef _WIN32_WCE
    fgJoystickClose( );
#endif

    fgState.Initialised = GL_FALSE;

    fgState.Position.X = -1;
    fgState.Position.Y = -1;
    fgState.Position.Use = GL_FALSE;

    fgState.Size.X = 240;
    fgState.Size.Y = 320;
    fgState.Size.Use = GL_TRUE;

    fgState.DisplayMode = GLUT_RGBA | GLUT_SINGLE | GLUT_DEPTH;

    fgState.ForceDirectContext  = GL_FALSE;
    fgState.TryDirectContext    = GL_TRUE;
    fgState.ForceIconic         = GL_FALSE;
    fgState.UseCurrentContext   = GL_FALSE;
    fgState.GLDebugSwitch       = GL_FALSE;
    fgState.XSyncSwitch         = GL_FALSE;
    fgState.ActionOnWindowClose = GLUT_ACTION_EXIT;
    fgState.ExecState           = GLUT_EXEC_STATE_INIT;

    fgState.IgnoreKeyRepeat = GL_TRUE;
    fgState.Modifiers       = 0xffffffff;

    fgState.GameModeSize.X  = 240;
    fgState.GameModeSize.Y  = 320;
    fgState.GameModeDepth   =  16;
    fgState.GameModeRefresh =  72;

    fgState.Time.Set = GL_FALSE;

    fgListInit( &fgState.Timers );
    fgListInit( &fgState.FreeTimers );

    fgState.IdleCallback = NULL;
    fgState.MenuStateCallback = ( FGCBMenuState )NULL;
    fgState.MenuStatusCallback = ( FGCBMenuStatus )NULL;

    fgState.SwapCount   = 0;
    fgState.SwapTime    = 0;
    fgState.FPSInterval = 0;

    if( fgState.ProgramName )
    {
        free( fgState.ProgramName );
        fgState.ProgramName = NULL;
    }
    
	__glDestroy();

#if TARGET_HOST_UNIX_X11

    /*
     * Make sure all X-client data we have created will be destroyed on
     * display closing
     */
    XSetCloseDownMode( fgDisplay.Display, DestroyAll );

    /*
     * Close the display connection, destroying all windows we have
     * created so far
     */
    XCloseDisplay( fgDisplay.Display );

#endif
}

/*
 * Everything inside the following #ifndef is copied from the X sources.
 */

#if TARGET_HOST_WIN32

/*

Copyright 1985, 1986, 1987,1998  The Open Group

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall
not be used in advertising or otherwise to promote the sale, use or
other dealings in this Software without prior written authorization
from The Open Group.

*/

#define NoValue         0x0000
#define XValue          0x0001
#define YValue          0x0002
#define WidthValue      0x0004
#define HeightValue     0x0008
#define AllValues       0x000F
#define XNegative       0x0010
#define YNegative       0x0020

/*
 *    XParseGeometry parses strings of the form
 *   "=<width>x<height>{+-}<xoffset>{+-}<yoffset>", where
 *   width, height, xoffset, and yoffset are unsigned integers.
 *   Example:  "=80x24+300-49"
 *   The equal sign is optional.
 *   It returns a bitmask that indicates which of the four values
 *   were actually found in the string.  For each value found,
 *   the corresponding argument is updated;  for each value
 *   not found, the corresponding argument is left unchanged. 
 */

static int
ReadInteger(char *string, char **NextString)
{
    register int Result = 0;
    int Sign = 1;
    
    if (*string == '+')
        string++;
    else if (*string == '-')
    {
        string++;
        Sign = -1;
    }
    for (; (*string >= '0') && (*string <= '9'); string++)
    {
        Result = (Result * 10) + (*string - '0');
    }
    *NextString = string;
    if (Sign >= 0)
        return Result;
    else
        return -Result;
}

static int XParseGeometry (
    const char *string,
    int *x,
    int *y,
    unsigned int *width,    /* RETURN */
    unsigned int *height)    /* RETURN */
{
    int mask = NoValue;
    register char *strind;
    unsigned int tempWidth = 0, tempHeight = 0;
    int tempX = 0, tempY = 0;
    char *nextCharacter;

    if ( (string == NULL) || (*string == '\0'))
      return mask;
    if (*string == '=')
        string++;  /* ignore possible '=' at beg of geometry spec */

    strind = (char *)string;
    if (*strind != '+' && *strind != '-' && *strind != 'x') {
        tempWidth = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter) 
            return 0;
        strind = nextCharacter;
        mask |= WidthValue;
    }

    if (*strind == 'x' || *strind == 'X') {
        strind++;
        tempHeight = ReadInteger(strind, &nextCharacter);
        if (strind == nextCharacter)
            return 0;
        strind = nextCharacter;
        mask |= HeightValue;
    }

    if ((*strind == '+') || (*strind == '-')) {
        if (*strind == '-') {
            strind++;
            tempX = -ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter)
                return 0;
            strind = nextCharacter;
            mask |= XNegative;
        }
        else
        {
            strind++;
            tempX = ReadInteger(strind, &nextCharacter);
            if (strind == nextCharacter)
                return 0;
            strind = nextCharacter;
        }
        mask |= XValue;
        if ((*strind == '+') || (*strind == '-')) {
            if (*strind == '-') {
                strind++;
                tempY = -ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter)
                    return 0;
                strind = nextCharacter;
                mask |= YNegative;
            }
            else
            {
                strind++;
                tempY = ReadInteger(strind, &nextCharacter);
                if (strind == nextCharacter)
                    return 0;
                strind = nextCharacter;
            }
            mask |= YValue;
        }
    }

    /* If strind isn't at the end of the string the it's an invalid
       geometry specification. */

    if (*strind != '\0') return 0;

    if (mask & XValue)
        *x = tempX;
    if (mask & YValue)
        *y = tempY;
    if (mask & WidthValue)
        *width = tempWidth;
    if (mask & HeightValue)
        *height = tempHeight;
    return mask;
}
#endif

/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Perform initialization. This usually happens on the program startup
 * and restarting after glutMainLoop termination...
 */
void FGAPIENTRY glutInit( int* pargc, char** argv )
{
    char* displayName = NULL;
    char* geometry = NULL;
    int i, j, argc = *pargc;

    if( fgState.Initialised )
        fgError( "illegal glutInit() reinitialization attempt" );

    if (pargc && *pargc && argv && *argv && **argv)
    {
        fgState.ProgramName = strdup (*argv);

        if( !fgState.ProgramName )
            fgError ("Could not allocate space for the program's name.");
    }

    fgCreateStructure( );

    fgElapsedTime( );

#ifndef _WIN32_WCE
    /* check if GLUT_FPS env var is set */
    {
        const char *fps = getenv( "GLUT_FPS" );
        if( fps )
        {
            sscanf( fps, "%d", &fgState.FPSInterval );
            if( fgState.FPSInterval <= 0 )
                fgState.FPSInterval = 5000;  /* 5000 milliseconds */
        }
    }

    displayName = getenv( "DISPLAY");

    for( i = 1; i < argc; i++ )
    {
        if( strcmp( argv[ i ], "-display" ) == 0 )
        {
            if( ++i >= argc )
                fgError( "-display parameter must be followed by display name" );

            displayName = argv[ i ];

            argv[ i - 1 ] = NULL;
            argv[ i     ] = NULL;
            ( *pargc ) -= 2;
        }
        else if( strcmp( argv[ i ], "-geometry" ) == 0 )
        {
            if( ++i >= argc )
                fgError( "-geometry parameter must be followed by window "
                         "geometry settings" );

            geometry = argv[ i ];

            argv[ i - 1 ] = NULL;
            argv[ i     ] = NULL;
            ( *pargc ) -= 2;
        }
        else if( strcmp( argv[ i ], "-direct" ) == 0)
        {
            if( ! fgState.TryDirectContext )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.ForceDirectContext = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-indirect" ) == 0 )
        {
            if( fgState.ForceDirectContext )
                fgError( "parameters ambiguity, -direct and -indirect "
                    "cannot be both specified" );

            fgState.TryDirectContext = GL_FALSE;
            argv[ i ] = NULL;
            (*pargc)--;
        }
        else if( strcmp( argv[ i ], "-iconic" ) == 0 )
        {
            fgState.ForceIconic = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-gldebug" ) == 0 )
        {
            fgState.GLDebugSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
        else if( strcmp( argv[ i ], "-sync" ) == 0 )
        {
            fgState.XSyncSwitch = GL_TRUE;
            argv[ i ] = NULL;
            ( *pargc )--;
        }
    }
#endif
    /*
     * Compact {argv}.
     */
    j = 2;
    for( i = 1; i < *pargc; i++, j++ )
    {
        if( argv[ i ] == NULL )
        {
            /* Guaranteed to end because there are "*pargc" arguments left */
            while ( argv[ j ] == NULL )
                j++;
            argv[ i ] = argv[ j ];
        }
    }

    /*
     * Have the display created now. As I am too lazy to implement
     * the program arguments parsing, we will have the DISPLAY
     * environment variable used for opening the X display:
     *
     * XXX The above comment is rather unclear.  We have just
     * XXX completed parsing of the program arguments for GLUT
     * XXX parameters.  We obviously canNOT parse the application-
     * XXX specific parameters.  Can someone re-write the above
     * XXX more clearly?
     */
    fgInitialize( displayName );

    /*
     * Geometry parsing deffered until here because we may need the screen
     * size.
     */

    if (geometry )
    {
        int mask = XParseGeometry( geometry,
                                   &fgState.Position.X, &fgState.Position.Y,
                                   &fgState.Size.X, &fgState.Size.Y );

        if( (mask & (WidthValue|HeightValue)) == (WidthValue|HeightValue) )
            fgState.Size.Use = GL_TRUE;

        if( mask & XNegative )
            fgState.Position.X += fgDisplay.ScreenWidth - fgState.Size.X;

        if( mask & YNegative )
            fgState.Position.Y += fgDisplay.ScreenHeight - fgState.Size.Y;

        if( (mask & (XValue|YValue)) == (XValue|YValue) )
            fgState.Position.Use = GL_TRUE;
    }
}

/*
 * Sets the default initial window position for new windows
 */
void FGAPIENTRY glutInitWindowPosition( int x, int y )
{
    fgState.Position.X = x;
    fgState.Position.Y = y;

    if( ( x >= 0 ) && ( y >= 0 ) )
        fgState.Position.Use = GL_TRUE;
    else
        fgState.Position.Use = GL_FALSE;
}

/*
 * Sets the default initial window size for new windows
 */
void FGAPIENTRY glutInitWindowSize( int width, int height )
{
    fgState.Size.X = width;
    fgState.Size.Y = height;

    if( ( width > 0 ) && ( height > 0 ) )
        fgState.Size.Use = GL_TRUE;
    else
        fgState.Size.Use = GL_FALSE;
}

/*
 * Sets the default display mode for all new windows
 */
void FGAPIENTRY glutInitDisplayMode( unsigned int displayMode )
{
    /*
     * We will make use of this value when creating a new OpenGL context...
     */
    fgState.DisplayMode = displayMode;
}


/* -- INIT DISPLAY STRING PARSING ------------------------------------------ */

static char* Tokens[] =
{
    "surface", "buffer", "red", "green", "blue", "alpha", 
	//"caveat", "id", 
	"depth",
	//"level", 
	//"renderable", 
	//"visual_style", 
	//"sample_buffers", "samples", 
	"stencil",
    //"transparent_type", "transparent_red", "transparent_green", "transparent_blue"
};

void FGAPIENTRY glutInitDisplayString( const char* displayMode )
{
    int glut_state_flag = 0 ;
    /*
     * Unpack a lot of options from a character string.  The options are
     * delimited by blanks or tabs.
     */
    char *token ;
    int len = strlen ( displayMode );
    char *buffer = strdup ( displayMode );

    token = strtok ( buffer, " \t" );
    while ( token )
    {
        /*
         * Process this token
         */
		int i;
        for ( i = 0; i < (sizeof(Tokens) / sizeof(char*)); i++ )
        {
            if ( strncmp ( token, Tokens[i], strlen(Tokens[i]) ) == 0 ) 
			{
				int p, param = EGL_DONT_CARE;
				p = strlen( Tokens[i] );	
				while ( token[p] == '=' || token[p] == '<' || token[p] == '>' ) p++;
				param = atoi( token + p );
				if ( param < -1 ) param = -1;
				fgEGLConfigAttribs[ i * 2 + 1 ] = param;
			}
		}
        token = strtok ( NULL, " \t" );
    }

    free ( buffer );

    /*
     * We will make use of this value when creating a new OpenGL context...
     */
    //fgState.DisplayMode = glut_state_flag;
}

/*** END OF FILE ***/
