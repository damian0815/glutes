/*
 * glutes_window.c
 *
 * Window management methods.
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
 * PAWEL W. OLSZTA BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"

#ifdef _WIN32_WCE
#	include <aygshell.h>
#endif

extern void __glCreateSurface(SFG_Window* w);
extern void __glDestroySurface(SFG_Window* w);
extern void __glCreateContext(SFG_Window* w);


/*
 * TODO BEFORE THE STABLE RELEASE:
 *
 *  fgChooseVisual()        -- OK, but what about glutInitDisplayString()?
 *  fgSetupPixelFormat      -- ignores the display mode settings
 *  fgOpenWindow()          -- check the Win32 version, -iconic handling!
 *  fgCloseWindow()         -- check the Win32 version
 *  glutCreateWindow()      -- Check when default position and size is {-1,-1}
 *  glutCreateSubWindow()   -- Check when default position and size is {-1,-1}
 *  glutDestroyWindow()     -- check the Win32 version
 *  glutSetWindow()         -- check the Win32 version
 *  glutGetWindow()         -- OK
 *  glutSetWindowTitle()    -- check the Win32 version
 *  glutSetIconTitle()      -- check the Win32 version
 *  glutShowWindow()        -- check the Win32 version
 *  glutHideWindow()        -- check the Win32 version
 *  glutIconifyWindow()     -- check the Win32 version
 *  glutReshapeWindow()     -- check the Win32 version
 *  glutPositionWindow()    -- check the Win32 version
 *  glutPushWindow()        -- check the Win32 version
 *  glutPopWindow()         -- check the Win32 version
 */

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */

/*
 * Chooses a visual basing on the current display mode settings
 */
#if TARGET_HOST_UNIX_X11

/*
 * Why is there a semi-colon in this #define?  The code
 * that uses the macro seems to always add more semicolons...
 */
#define ATTRIB(a) attributes[where++]=a;
#define ATTRIB_VAL(a,v) {ATTRIB(a); ATTRIB(v);}

#if defined(USE_EGL)
EGLConfig fgChooseConfig( void )
{
    EGLConfig config;
    EGLint    found_configs = 0;
    int attributes[ 32 ];
    int where = 0;

    if( fgState.DisplayMode & GLUT_INDEX )
    {
        fgError("Indexed color buffers not supported.");
        return 0;
    }
    else
    {
        ATTRIB_VAL( EGL_RED_SIZE,   1 );
        ATTRIB_VAL( EGL_GREEN_SIZE, 1 );
        ATTRIB_VAL( EGL_BLUE_SIZE,  1 );
        if( fgState.DisplayMode & GLUT_ALPHA )
            ATTRIB_VAL( EGL_ALPHA_SIZE, 1 );
    }

    if( fgState.DisplayMode & GLUT_DEPTH )
        ATTRIB_VAL( EGL_DEPTH_SIZE, 1 );

    if( fgState.DisplayMode & GLUT_STENCIL )
        ATTRIB_VAL( EGL_STENCIL_SIZE, 1 );

    ATTRIB_VAL( EGL_SURFACE_TYPE,   EGL_WINDOW_BIT );

    /*
     * Push a null at the end of the list
     */
    ATTRIB( EGL_NONE );

    if (eglChooseConfig(fgDisplay.eglDisplay, attributes,
                        &config, 1, &found_configs) == EGL_FALSE ||
        found_configs == 0)
    {
        fgError("Unable to find a suitable config.");
        return 0;
    }
    return config;
}
#endif /* USE_EGL */

XVisualInfo* fgChooseVisual( void )
{
    /*
     * First we have to process the display mode settings...
     */
#if defined(USE_EGL)
    EGLConfig config = fgChooseConfig();
    VisualID  visual_id = 0;

    if (!config)
    {
        return NULL;
    }

    if (eglGetConfigAttrib(fgDisplay.eglDisplay, config, EGL_NATIVE_VISUAL_ID,
                           (EGLint*)&visual_id) == EGL_FALSE ||
        !visual_id)
    {
        /* Use the default visual when all else fails */
        XVisualInfo vi_in;
        int out_count;
        vi_in.screen = fgDisplay.Screen;

        return XGetVisualInfo(fgDisplay.Display, VisualScreenMask,
                              &vi_in, &out_count);
    }
    else
    {
        XVisualInfo vi_in;
        int out_count;

        vi_in.screen   = fgDisplay.Screen;
        vi_in.visualid = visual_id;
        return XGetVisualInfo(fgDisplay.Display, VisualScreenMask|VisualIDMask,
                              &vi_in, &out_count);
    }
#else /* USE_EGL */
#define BUFFER_SIZES 6
    int bufferSize[BUFFER_SIZES] = { 16, 12, 8, 4, 2, 1 };
    GLboolean wantIndexedMode = GL_FALSE;
    int attributes[ 32 ];
    int where = 0;

    if( fgState.DisplayMode & GLUT_INDEX )
    {
        ATTRIB_VAL( GLX_BUFFER_SIZE, 8 );
        wantIndexedMode = GL_TRUE;
    }
    else
    {
        ATTRIB( GLX_RGBA );
        ATTRIB_VAL( GLX_RED_SIZE,   1 );
        ATTRIB_VAL( GLX_GREEN_SIZE, 1 );
        ATTRIB_VAL( GLX_BLUE_SIZE,  1 );
        if( fgState.DisplayMode & GLUT_ALPHA )
            ATTRIB_VAL( GLX_ALPHA_SIZE, 1 );
    }

    if( fgState.DisplayMode & GLUT_DOUBLE )
        ATTRIB( GLX_DOUBLEBUFFER );

    if( fgState.DisplayMode & GLUT_STEREO )
        ATTRIB( GLX_STEREO );

    if( fgState.DisplayMode & GLUT_DEPTH )
        ATTRIB_VAL( GLX_DEPTH_SIZE, 1 );

    if( fgState.DisplayMode & GLUT_STENCIL )
        ATTRIB_VAL( GLX_STENCIL_SIZE, 1 );

    if( fgState.DisplayMode & GLUT_ACCUM )
    {
        ATTRIB_VAL( GLX_ACCUM_RED_SIZE,   1 );
        ATTRIB_VAL( GLX_ACCUM_GREEN_SIZE, 1 );
        ATTRIB_VAL( GLX_ACCUM_BLUE_SIZE,  1 );
        if( fgState.DisplayMode & GLUT_ALPHA )
            ATTRIB_VAL( GLX_ACCUM_ALPHA_SIZE, 1 );
    }

    /*
     * Push a null at the end of the list
     */
    ATTRIB( None );

    if( ! wantIndexedMode )
        return glXChooseVisual( fgDisplay.Display, fgDisplay.Screen,
                                attributes );
    else
    {
        XVisualInfo* visualInfo;
        int i;

        /*
         * In indexed mode, we need to check how many bits of depth can we
         * achieve.  We do this by trying each possibility from the list
         * given in the {bufferSize} array.  If we match, we return to caller.
         */
        for( i=0; i<BUFFER_SIZES; i++ )
        {
            attributes[ 1 ] = bufferSize[ i ];
            visualInfo = glXChooseVisual( fgDisplay.Display, fgDisplay.Screen,
                                          attributes );
            if( visualInfo != NULL )
                return visualInfo;
        }
        return NULL;
    }
#endif /* USE_EGL */
}
#endif

/*
 * Setup the pixel format for a Win32 window
 */
#if TARGET_HOST_WIN32
GLboolean fgSetupPixelFormat( SFG_Window* window, GLboolean checkOnly,
                              unsigned char layer_type )
{
#ifdef _WIN32_WCE
	return GL_TRUE;
#else

    PIXELFORMATDESCRIPTOR* ppfd, pfd;
    int flags, pixelformat;

    glutes_return_val_if_fail( window != NULL, 0 );
    flags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    if( fgState.DisplayMode & GLUT_DOUBLE )
        flags |= PFD_DOUBLEBUFFER;

#pragma message( "fgSetupPixelFormat(): there is still some work to do here!" )

    /*
     * Specify which pixel format do we opt for...
     */
    pfd.nSize           = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion        = 1;
    pfd.dwFlags         = flags;
    pfd.iPixelType      = PFD_TYPE_RGBA;
    pfd.cColorBits      = 24;
    pfd.cRedBits        = 0;
    pfd.cRedShift       = 0;
    pfd.cGreenBits      = 0;
    pfd.cGreenShift     = 0;
    pfd.cBlueBits       = 0;
    pfd.cBlueShift      = 0;
    pfd.cAlphaBits      = 0;
    pfd.cAlphaShift     = 0;
    pfd.cAccumBits      = 0;
    pfd.cAccumRedBits   = 0;
    pfd.cAccumGreenBits = 0;
    pfd.cAccumBlueBits  = 0;
    pfd.cAccumAlphaBits = 0;
#if 0
    pfd.cDepthBits      = 32;
    pfd.cStencilBits    = 0;
#else
    pfd.cDepthBits      = 24;
    pfd.cStencilBits    = 8;
#endif
    pfd.cAuxBuffers     = 0;
    pfd.iLayerType      = layer_type;
    pfd.bReserved       = 0;
    pfd.dwLayerMask     = 0;
    pfd.dwVisibleMask   = 0;
    pfd.dwDamageMask    = 0;

    pfd.cColorBits = (BYTE) GetDeviceCaps( window->Window.Device, BITSPIXEL );
    ppfd = &pfd;
    
    pixelformat = ChoosePixelFormat( window->Window.Device, ppfd );
    if( pixelformat == 0 )
        return GL_FALSE;

    if( checkOnly )
        return GL_TRUE;
    return SetPixelFormat( window->Window.Device, pixelformat, ppfd );
#endif
}
#endif

/*
 * Sets the OpenGL context and the fgStructure "Current Window" pointer to
 * the window structure passed in.
 */
void fgSetWindow ( SFG_Window *window )
{
#if TARGET_HOST_UNIX_X11
#  if defined(USE_EGL)
    if ( window )
        eglMakeCurrent(
            fgDisplay.eglDisplay,
            window->Window.Surface,
            window->Window.Surface,
            window->Window.Context
        );
#  else /* USE_EGL */
    if ( window )
        glXMakeCurrent(
            fgDisplay.Display,
            window->Window.Handle,
            window->Window.Context
        );
#  endif /* USE_EGL */
#elif TARGET_HOST_WIN32
    if( fgStructure.Window )
        ReleaseDC( fgStructure.Window->Window.Handle,
                   fgStructure.Window->Window.Device );

    if ( window )
    {
        /*window->Window.Device = GetDC( window->Window.Handle );
        wglMakeCurrent( 
            window->Window.Device, 
            window->Window.Context
        );*/
		if(!
		eglMakeCurrent(fgDisplay.eglDisplay, 
			window->Window.Surface, window->Window.Surface, window->Window.Context))
		{
			//fgError("Unable to make current OpenGL|ES context\n(ErrCode: %d)!", eglGetError());
		}
    }
#endif
    fgStructure.Window = window;
}

#if TARGET_HOST_WIN32
void 
UpdateWindowPosition(HWND hWnd);
#endif

/*
 * Opens a window. Requires a SFG_Window object created and attached
 * to the freeglut structure. OpenGL context is created here.
 */
void fgOpenWindow( SFG_Window* window, const char* title,
                   int x, int y, int w, int h,
                   GLboolean gameMode, GLboolean isSubWindow )
{
#if TARGET_HOST_UNIX_X11
    XSetWindowAttributes winAttr;
    XTextProperty textProperty;
    XSizeHints sizeHints;
    XWMHints wmHints;
    unsigned long mask;

    glutes_assert_ready;

    /*
     * XXX fgChooseVisual() is a common part of all three.
     * XXX With a little thought, we should be able to greatly
     * XXX simplify this.
     */
    if ( !window->IsMenu )
      window->Window.VisualInfo = fgChooseVisual();
    else if ( fgStructure.MenuContext )
        window->Window.VisualInfo = fgChooseVisual();
    else
    {
        unsigned int current_DisplayMode = fgState.DisplayMode ;
        fgState.DisplayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH ;
        window->Window.VisualInfo = fgChooseVisual();
        fgState.DisplayMode = current_DisplayMode ;
    }

    if ( ! window->Window.VisualInfo )
    {
        /*
         * The "fgChooseVisual" returned a null meaning that the visual
         * context is not available.
         * Try a couple of variations to see if they will work.
         */
        if ( ! ( fgState.DisplayMode & GLUT_DOUBLE ) )
        {
            fgState.DisplayMode |= GLUT_DOUBLE ;
            window->Window.VisualInfo = fgChooseVisual();
            fgState.DisplayMode &= ~GLUT_DOUBLE ;
        }
        
        /*
         * GLUT also checks for multi-sampling, but I don't see that
         * anywhere else in FREEGLUT so I won't bother with it for the moment.
         */
    }

    assert( window->Window.VisualInfo != NULL );

    /*
     * XXX HINT: the masks should be updated when adding/removing callbacks.
     * XXX       This might speed up message processing. Is that true?
     * XXX
     * XXX A: Not appreciably, but it WILL make it easier to debug.
     * XXX    Try tracing old GLUT and try tracing freeglut.  Old GLUT
     * XXX    turns off events that it doesn't need and is a whole lot
     * XXX    more pleasant to trace.  (Hint: Think mouse-motion!)
     * XXX
     * XXX    It may make a difference in networked environments or on
     * XXX    some very slow systems, but I think that that is secondary
     * XXX    to making debugging easier.
     */
    winAttr.event_mask        = StructureNotifyMask | SubstructureNotifyMask |
        ExposureMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask |
        KeyRelease | VisibilityChangeMask | EnterWindowMask | LeaveWindowMask |
        PointerMotionMask | ButtonMotionMask;
    winAttr.background_pixmap = None;
    winAttr.background_pixel  = 0;
    winAttr.border_pixel      = 0;

    winAttr.colormap = XCreateColormap(
        fgDisplay.Display, fgDisplay.RootWindow,
        window->Window.VisualInfo->visual, AllocNone
    );

    mask = CWBackPixmap | CWBorderPixel | CWColormap | CWEventMask;

    if ( window->IsMenu )
    {
        winAttr.override_redirect = True;
        mask |= CWOverrideRedirect;
    }

    window->Window.Handle = XCreateWindow(
        fgDisplay.Display,
        window->Parent == NULL ? fgDisplay.RootWindow :
                                 window->Parent->Window.Handle,
        x, y, w, h, 0,
        window->Window.VisualInfo->depth, InputOutput,
        window->Window.VisualInfo->visual, mask,
        &winAttr
    );

#if defined(USE_EGL)
    window->Window.eglConfig = fgChooseConfig();

    /*
     *  Create the EGL window surface
     */
    window->Window.Surface     = eglCreateWindowSurface(fgDisplay.eglDisplay, window->Window.eglConfig, window->Window.Handle, NULL);
    window->Window.SurfaceType = EGL_WINDOW_BIT;

    if (window->Window.Surface == EGL_NO_SURFACE)
    {
        fgError("Window surface creation failed.");
        return;
    }

    if ( window->IsMenu )
    {
        /*
         * If there isn't already an OpenGL rendering context for menu
         * windows, make one
         */
        if ( !fgStructure.MenuContext )
        {
            fgStructure.MenuContext =
                (SFG_MenuContext *)malloc ( sizeof(SFG_MenuContext) );
            fgStructure.MenuContext->VisualInfo = window->Window.VisualInfo;
            fgStructure.MenuContext->Context = eglCreateContext(fgDisplay.eglDisplay, window->Window.eglConfig, 0, NULL);
        }

        window->Window.Context = eglCreateContext(fgDisplay.eglDisplay, window->Window.eglConfig, 0, NULL);
    }
    else if ( fgState.UseCurrentContext )
    {
      window->Window.Context = eglGetCurrentContext();

      if ( ! window->Window.Context )
        window->Window.Context = eglCreateContext(fgDisplay.eglDisplay, window->Window.eglConfig, 0, NULL);
    }
    else
        window->Window.Context = eglCreateContext(fgDisplay.eglDisplay, window->Window.eglConfig, 0, NULL);


    eglMakeCurrent(
        fgDisplay.eglDisplay,
        window->Window.Surface,
        window->Window.Surface,
        window->Window.Context
    );
#else /* defined(USE_EGL) */
    /*
     * The GLX context creation, possibly trying the direct context rendering
     *  or else use the current context if the user has so specified
     */
    if ( window->IsMenu )
    {
        /*
         * If there isn't already an OpenGL rendering context for menu
         * windows, make one
         */
        if ( !fgStructure.MenuContext )
        {
            fgStructure.MenuContext =
                (SFG_MenuContext *)malloc ( sizeof(SFG_MenuContext) );
            fgStructure.MenuContext->VisualInfo = window->Window.VisualInfo;
            fgStructure.MenuContext->Context = glXCreateContext(
                fgDisplay.Display, fgStructure.MenuContext->VisualInfo,
                NULL, fgState.ForceDirectContext | fgState.TryDirectContext
            );
        }

/*      window->Window.Context = fgStructure.MenuContext->Context ; */
        window->Window.Context = glXCreateContext(
            fgDisplay.Display, window->Window.VisualInfo,
            NULL, fgState.ForceDirectContext | fgState.TryDirectContext
        );
    }
    else if ( fgState.UseCurrentContext )
    {
      window->Window.Context = glXGetCurrentContext();

      if ( ! window->Window.Context )
        window->Window.Context = glXCreateContext(
            fgDisplay.Display, window->Window.VisualInfo,
            NULL, fgState.ForceDirectContext | fgState.TryDirectContext
        );
    }
    else
        window->Window.Context = glXCreateContext(
            fgDisplay.Display, window->Window.VisualInfo,
            NULL, fgState.ForceDirectContext | fgState.TryDirectContext
        );

    if( fgState.ForceDirectContext &&
        !glXIsDirect( fgDisplay.Display, window->Window.Context ) )
        fgError( "unable to force direct context rendering for window '%s'",
                 title );

    glXMakeCurrent(
        fgDisplay.Display,
        window->Window.Handle,
        window->Window.Context
    );
#endif /* defined(USE_EGL) */

    /*
     * XXX Assume the new window is visible by default
     * XXX Is this a  safe assumption?
     */
    window->State.Visible = GL_TRUE;

    sizeHints.flags = 0;
    if ( fgState.Position.Use )
        sizeHints.flags |= USPosition;
    if ( fgState.Size.Use )
        sizeHints.flags |= USSize;

    /*
     * Fill in the size hints values now (the x, y, width and height
     * settings are obsolote, are there any more WMs that support them?)
     * Unless the X servers actually stop supporting these, we should
     * continue to fill them in.  It is *not* our place to tell the user
     * that they should replace a window manager that they like, and which
     * works, just because *we* think that it's not "modern" enough.
     */
    sizeHints.x      = x;
    sizeHints.y      = y;
    sizeHints.width  = w;
    sizeHints.height = h;

    wmHints.flags = StateHint;
    wmHints.initial_state = fgState.ForceIconic ? IconicState : NormalState;

    /*
     * Prepare the window and iconified window names...
     */
    XStringListToTextProperty( (char **) &title, 1, &textProperty );

    XSetWMProperties(
        fgDisplay.Display,
        window->Window.Handle,
        &textProperty,
        &textProperty,
        0,
        0,
        &sizeHints,
        &wmHints,
        NULL
    );
    XSetWMProtocols( fgDisplay.Display, window->Window.Handle,
                     &fgDisplay.DeleteWindow, 1 );
    XMapWindow( fgDisplay.Display, window->Window.Handle );

#elif TARGET_HOST_WIN32

    WNDCLASS wc;
    int flags;
    ATOM atom;

    glutes_assert_ready;
    
    /*
     * Grab the window class we have registered on glutInit():
     */
    atom = GetClassInfo( fgDisplay.Instance, TEXT("GLUT|ES"), &wc );
    assert( atom != 0 );
    
    if( gameMode )
    {
        assert( window->Parent == NULL );

        /*
         * Set the window creation flags appropriately to make the window
         * entirely visible:
         */
        flags = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
    }
    else
    {
        if ( ( ! isSubWindow ) && ( ! window->IsMenu ) )
        {
            /*
             * Update the window dimensions, taking account of window
             * decorations.  "freeglut" is to create the window with the
             * outside of its border at (x,y) and with dimensions (w,h).
             */
#ifndef _WIN32_WCE
            w += (GetSystemMetrics( SM_CXSIZEFRAME ) )*2;
            h += (GetSystemMetrics( SM_CYSIZEFRAME ) )*2 +
                GetSystemMetrics( SM_CYCAPTION );
#endif
        }

        if( ! fgState.Position.Use )
        {
            x = CW_USEDEFAULT;
            y = CW_USEDEFAULT;
        }
        if( ! fgState.Size.Use )
        {
            w = CW_USEDEFAULT;
            h = CW_USEDEFAULT;
        }

        /*
         * There's a small difference between creating the top, child and
         * game mode windows
         */
#ifdef _WIN32_WCE 
		flags = WS_VISIBLE;
		if ( !window->IsMenu )
		{
			x = CW_USEDEFAULT;
			y = CW_USEDEFAULT;
			w = CW_USEDEFAULT;
			h = CW_USEDEFAULT;
		}
#else
        flags = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
#endif


        if ( window->IsMenu )
#ifdef _WIN32_WCE
			;//flags |= WS_POPUP;;
#else
            flags |= WS_POPUP ;

        else if( window->Parent == NULL )
            flags |= WS_OVERLAPPEDWINDOW;
        else
            flags |= WS_CHILD;
#endif//		flags |= WS_CHILD;
    }

#ifdef UNICODE
	{
	WCHAR *wtitle = NULL;
	if(title)
	{	
		wtitle = (WCHAR*)malloc((strlen(title) + 1) * sizeof(WCHAR));
		mbstowcs(wtitle, title, strlen(title) + 1);
	}
	window->Window.Handle = CreateWindow( 
        TEXT("GLUT|ES"),
		wtitle,
        flags,
        x, y, w, h,
        (HWND) window->Parent == NULL ? NULL : window->Parent->Window.Handle,
        (HMENU) NULL,
        fgDisplay.Instance,
        (LPVOID) window
    );
	if(wtitle)
		free(wtitle);
	}
#else
    window->Window.Handle = CreateWindow( 
        TEXT("GLUT|ES"),
		title,
        flags,
        x, y, w, h,
        (HWND) window->Parent == NULL ? NULL : window->Parent->Window.Handle,
        (HMENU) NULL,
        fgDisplay.Instance,
        (LPVOID) window
    );
#endif
    if( !( window->Window.Handle ) )
        fgError( "Failed to create a window (%s) %d!", title, GetLastError() );

    window->Window.DoubleBuffered =
        ( fgState.DisplayMode & GLUT_DOUBLE ) ? 1 : 0;

    if ( ! window->Window.DoubleBuffered )
    {
		/*
		glDrawBuffer ( GL_FRONT );
        glReadBuffer ( GL_FRONT );
		*/
    }

	//fgSetupPixelFormat( window, GL_FALSE, PFD_MAIN_PLANE );
	
	window->Window.Surface = EGL_NO_SURFACE;
	window->Window.Context = EGL_NO_CONTEXT;

	__glCreateContext(window);
	__glCreateSurface(window);
	
	/*
	if( ! fgState.UseCurrentContext )
	window->Window.Context =
	wglCreateContext( window->Window.Device );
	else
	{
	window->Window.Context = wglGetCurrentContext( );
	if( ! window->Window.Context )
	window->Window.Context =
	wglCreateContext( window->Window.Device );
	
	}*/
	
#ifdef _WIN32_WCE
	ShowWindow( window->Window.Handle,
                fgState.ForceIconic ? SW_HIDE : SW_SHOW );
	SetForegroundWindow((HWND)((ULONG)window->Window.Handle | 0x1));
	if(!window->IsMenu)
	{
		UpdateWindowPosition(window->Window.Handle);
	}
#else
	ShowWindow( window->Window.Handle,
		fgState.ForceIconic ? SW_SHOWMINIMIZED : SW_SHOW );
#endif
    UpdateWindow( window->Window.Handle );
    ShowCursor( TRUE );  /* XXX Old comments say "hide cusror"! */

#endif
	fgSetWindow( window );

#if !defined(USE_GLMENU)	
	window->ButtonUses = 0;
#endif
}


/*
 * Closes a window, destroying the frame and OpenGL context
 */
void fgCloseWindow( SFG_Window* window )
{
    glutes_assert_ready;

#if TARGET_HOST_UNIX_X11

#if !defined(USE_EGL)
    glXDestroyContext( fgDisplay.Display, window->Window.Context );
#endif
    XDestroyWindow( fgDisplay.Display, window->Window.Handle );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    /*
     * Make sure we don't close a window with current context active
     */
    if( fgStructure.Window == window )
        //wglMakeCurrent( NULL, NULL );
		eglMakeCurrent(fgDisplay.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

    /*
     * Step through the list of windows.  If the rendering context
     * is not being used by another window, then we delete it.
     */
    {
        int used = FALSE ;
        SFG_Window *iter ;

        for( iter = (SFG_Window *)fgStructure.Windows.First;
             iter;
             iter = (SFG_Window *)iter->Node.Next )
        {
            if( ( iter->Window.Context == window->Window.Context ) &&
                ( iter != window ) )
                used = TRUE;
        }

        if( ! used )
		{
			//__glDestroySurface(window):
			//eglMakeCurrent(egldpy, EGL_NO_SURFACE, EGL_NO_SURFACE, window->Window.Context);
			eglDestroySurface(fgDisplay.eglDisplay, window->Window.Surface);
			//eglDestroyContext(fgDisplay.eglDisplay, window->Window.Context);
            //wglDeleteContext( window->Window.Context );
		}
    }

    DestroyWindow( window->Window.Handle );
#endif
}


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Creates a new top-level freeglut window
 */
int FGAPIENTRY glutCreateWindow( const char* title )
{
    return fgCreateWindow( NULL, title, fgState.Position.X, fgState.Position.Y,
                           fgState.Size.X, fgState.Size.Y, GL_FALSE,
                           GL_FALSE )->ID;
}

/*
 * This function creates a sub window.
 */
int FGAPIENTRY glutCreateSubWindow( int parentID, int x, int y, int w, int h )
{
    SFG_Window* window = NULL;
    SFG_Window* parent = NULL;

    glutes_assert_ready;
    parent = fgWindowByID( parentID );
    glutes_return_val_if_fail( parent != NULL, 0 );
    window = fgCreateWindow( parent, "", x, y, w, h, GL_FALSE, GL_FALSE );
    return window->ID;
}

/*
 * Destroys a window and all of its subwindows
 */
void FGAPIENTRY glutDestroyWindow( int windowID )
{
    SFG_Window* window = fgWindowByID( windowID );
    glutes_return_if_fail( window != NULL );
    {
        fgExecutionState ExecState = fgState.ExecState;
        fgAddToWindowDestroyList( window );
        fgState.ExecState = ExecState;
    }
}

/*
 * This function selects the current window
 */
void FGAPIENTRY glutSetWindow( int ID )
{
    SFG_Window* window = NULL;

    glutes_assert_ready;
    if( fgStructure.Window != NULL )
        if( fgStructure.Window->ID == ID )
            return;

    window = fgWindowByID( ID );
    if( window == NULL )
    {
        fgWarning( "glutSetWindow(): window ID %i not found!", ID );
        return;
    }

    fgSetWindow( window );
}

/*
 * This function returns the ID number of the current window, 0 if none exists
 */
int FGAPIENTRY glutGetWindow( void )
{
    glutes_assert_ready;
    if( fgStructure.Window == NULL )
        return 0;
    return fgStructure.Window->ID;
}

/*
 * This function makes the current window visible
 */
void FGAPIENTRY glutShowWindow( void )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11

    XMapWindow( fgDisplay.Display, fgStructure.Window->Window.Handle );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    ShowWindow( fgStructure.Window->Window.Handle, SW_SHOW );

#endif

    fgStructure.Window->State.Redisplay = GL_TRUE;
	__glCreateSurface(fgStructure.Window);
}

/*
 * This function hides the current window
 */
void FGAPIENTRY glutHideWindow( void )
{
    glutes_assert_ready;
    glutes_assert_window;

	__glDestroySurface(fgStructure.Window);

#if TARGET_HOST_UNIX_X11

    if( fgStructure.Window->Parent == NULL )
        XWithdrawWindow( fgDisplay.Display, fgStructure.Window->Window.Handle,
                         fgDisplay.Screen );
    else
        XUnmapWindow( fgDisplay.Display, fgStructure.Window->Window.Handle );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    ShowWindow( fgStructure.Window->Window.Handle, SW_HIDE );

#endif

    fgStructure.Window->State.Redisplay = GL_FALSE;
}

/*
 * Iconify the current window (top-level windows only)
 */
void FGAPIENTRY glutIconifyWindow( void )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11

    XIconifyWindow( fgDisplay.Display, fgStructure.Window->Window.Handle,
                    fgDisplay.Screen );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    ShowWindow( fgStructure.Window->Window.Handle, SW_MINIMIZE );

#endif

    fgStructure.Window->State.Redisplay = GL_FALSE;
}

/*
 * Set the current window's title
 */
void FGAPIENTRY glutSetWindowTitle( const char* title )
{
    glutes_assert_ready;
    glutes_assert_window;
    if( fgStructure.Window->Parent != NULL )
        return;

#if TARGET_HOST_UNIX_X11

    {
        XTextProperty text;
        
        text.value = (unsigned char *) title;
        text.encoding = XA_STRING;
        text.format = 8;
        text.nitems = strlen( title );
        
        XSetWMName(
            fgDisplay.Display,
            fgStructure.Window->Window.Handle,
            &text
        );
        
        XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */
    }

#elif TARGET_HOST_WIN32
#ifdef UNICODE
	{
		WCHAR *wtitle = (WCHAR*)malloc((strlen(title) + 1) * sizeof(WCHAR));
		mbstowcs(wtitle, title, strlen(title) + 1);
		SetWindowText( fgStructure.Window->Window.Handle, wtitle );
		free(wtitle);
	}
#else
	SetWindowText( fgStructure.Window->Window.Handle, title );
#endif

#endif

}

/*
 * Set the current window's iconified title
 */
void FGAPIENTRY glutSetIconTitle( const char* title )
{
    glutes_assert_ready;
    glutes_assert_window;

    if( fgStructure.Window->Parent != NULL )
        return;

#if TARGET_HOST_UNIX_X11

    {
        XTextProperty text;
        
        text.value = (unsigned char *) title;
        text.encoding = XA_STRING;
        text.format = 8;
        text.nitems = strlen( title );

        XSetWMIconName(
            fgDisplay.Display,
            fgStructure.Window->Window.Handle,
            &text
        );

        XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */
    }

#elif TARGET_HOST_WIN32
#ifdef UNICODE
	{
		WCHAR *wtitle = (WCHAR*)malloc((strlen(title) + 1) * sizeof(WCHAR));
		mbstowcs(wtitle, title, strlen(title) + 1);
		SetWindowText( fgStructure.Window->Window.Handle, wtitle );
		free(wtitle);
	}
#else
	SetWindowText( fgStructure.Window->Window.Handle, title );
#endif
#endif

}

/*
 * Change the current window's size
 */
void FGAPIENTRY glutReshapeWindow( int width, int height )
{
    glutes_assert_ready;
    glutes_assert_window;

    fgStructure.Window->State.NeedToResize = GL_TRUE;
    fgStructure.Window->State.Width  = width ;
    fgStructure.Window->State.Height = height;
}

/*
 * Change the current window's position
 */
void FGAPIENTRY glutPositionWindow( int x, int y )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11

    XMoveWindow( fgDisplay.Display, fgStructure.Window->Window.Handle, x, y );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    {
        RECT winRect;
        
        GetWindowRect( fgStructure.Window->Window.Handle, &winRect );
        MoveWindow(
            fgStructure.Window->Window.Handle,
            x,
            y,
            winRect.right - winRect.left,
            winRect.bottom - winRect.top,
            TRUE
        );
    }

#endif

}

/*
 * Lowers the current window (by Z order change)
 */
void FGAPIENTRY glutPushWindow( void )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11

    XLowerWindow( fgDisplay.Display, fgStructure.Window->Window.Handle );

#elif TARGET_HOST_WIN32

    SetWindowPos(
        fgStructure.Window->Window.Handle,
        HWND_BOTTOM,
        0, 0, 0, 0,
        SWP_NOSIZE | SWP_NOMOVE
    );

#endif

}

/*
 * Raises the current window (by Z order change)
 */
void FGAPIENTRY glutPopWindow( void )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11

    XRaiseWindow( fgDisplay.Display, fgStructure.Window->Window.Handle );

#elif TARGET_HOST_WIN32

    SetWindowPos(
        fgStructure.Window->Window.Handle,
        HWND_TOP,
        0, 0, 0, 0,
        SWP_NOSIZE | SWP_NOMOVE
    );

#endif

}

/*
 * Resize the current window so that it fits the whole screen
 */
void FGAPIENTRY glutFullScreen( void )
{
    glutes_assert_ready;
    glutes_assert_window;

#if TARGET_HOST_UNIX_X11
    {
        int x, y;
        Window w;

        XMoveResizeWindow(
            fgDisplay.Display,
            fgStructure.Window->Window.Handle,
            0, 0,
            fgDisplay.ScreenWidth,
            fgDisplay.ScreenHeight
        );

        XFlush( fgDisplay.Display ); /* This is needed */

        XTranslateCoordinates(
            fgDisplay.Display,
            fgStructure.Window->Window.Handle,
            fgDisplay.RootWindow,
            0, 0, &x, &y, &w
        );

        if (x || y)
        {
            XMoveWindow(
                fgDisplay.Display,
                fgStructure.Window->Window.Handle,
                -x, -y
            );
            XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */
        }
    }
#elif TARGET_HOST_WIN32
    {
        RECT rect;

        /* For fullscreen mode, force the top-left corner to 0,0
         * and adjust the window rectangle so that the client area
         * covers the whole screen.
         */

        rect.left   = 0;
        rect.top    = 0;
        rect.right  = fgDisplay.ScreenWidth;
        rect.bottom = fgDisplay.ScreenHeight;

#ifdef _WIN32_WCE
        AdjustWindowRectEx ( &rect, WS_CLIPSIBLINGS |
                                  WS_CLIPCHILDREN, FALSE, 0 );
#else
		AdjustWindowRect ( &rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
                                  WS_CLIPCHILDREN, FALSE );
#endif

        /*
         * SWP_NOACTIVATE	Do not activate the window
         * SWP_NOOWNERZORDER	Do not change position in z-order
         * SWP_NOSENDCHANGING	Supress WM_WINDOWPOSCHANGING message
         * SWP_NOZORDER		Retains the current Z order (ignore 2nd param)
         */

        SetWindowPos( fgStructure.Window->Window.Handle,
                      HWND_TOP,
                      rect.left,
                      rect.top,
                      rect.right  - rect.left,
                      rect.bottom - rect.top,
                      SWP_NOACTIVATE | SWP_NOOWNERZORDER | 
#ifndef _WIN32_WCE
					  SWP_NOSENDCHANGING |
#endif
                      SWP_NOZORDER
                    );
#ifdef _WIN32_WCE
		SHFullScreen(fgStructure.Window->Window.Handle, 
			SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
		SetForegroundWindow(fgStructure.Window->Window.Handle);
#endif
    }
#endif
}

/*
 * A.Donev: Set and retrieve the window's user data
 */
void* FGAPIENTRY glutGetWindowData( void )
{
    return fgStructure.Window->UserData;
}

void FGAPIENTRY glutSetWindowData(void* data)
{
    fgStructure.Window->UserData=data;
}

/*** END OF FILE ***/
