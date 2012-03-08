/*
 * glutes_main.c
 *
 * The windows message processing methods.
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

#ifdef _WIN32_WCE
#	include <aygshell.h>
#endif

#include <limits.h>
#if TARGET_HOST_UNIX_X11
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#elif TARGET_HOST_WIN32
#endif

#ifndef MAX
#define MAX(a,b) (((a)>(b)) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b))
#endif


/*
 * TODO BEFORE THE STABLE RELEASE:
 *
 * There are some issues concerning window redrawing under X11, and maybe
 * some events are not handled. The Win32 version lacks some more features,
 * but seems acceptable for not demanding purposes.
 *
 * Need to investigate why the X11 version breaks out with an error when
 * closing a window (using the window manager, not glutDestroyWindow)...
 */

/* -- PRIVATE FUNCTIONS ---------------------------------------------------- */
void __glCreateSurface(SFG_Window* w);

#ifdef _WIN32_WCE
void 
UpdateWindowPosition(HWND hWnd)
{
	{
	int cx, cy;
	SIPINFO si;
	// Query the sip state and size our window appropriately.
	memset (&si, 0, sizeof (si));
	si.cbSize = sizeof (si);
	SHSipInfo(SPI_GETSIPINFO, 0, (PVOID)&si, FALSE); 
	cx = si.rcVisibleDesktop.right - si.rcVisibleDesktop.left;
	cy = si.rcVisibleDesktop.bottom - si.rcVisibleDesktop.top;
	
	// If the sip is not shown, or showing but not docked, the
	// desktop rect doesn’t include the height of the menu bar.
	if(!(si.fdwFlags & SIPF_ON) ||
		((si.fdwFlags & SIPF_ON) && 
		!(si.fdwFlags & SIPF_DOCKED))) 
		cy -= GetSystemMetrics(SM_CYCAPTION);  

	/*SetWindowPos(hWnd, NULL, 0, GetSystemMetrics(SM_CYCAPTION), cx, cy, 
		SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);*/

	SHFullScreen(hWnd, SHFS_HIDETASKBAR | SHFS_HIDESIPBUTTON | SHFS_HIDESTARTICON);
	
	MoveWindow(hWnd, 0, 0,  GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN), TRUE);
	SetForegroundWindow((HWND)((ULONG)hWnd | 0x00000001));
	}
}
#endif

/*
 * Handle a window configuration change. When no reshape
 * callback is hooked, the viewport size is updated to
 * match the new window size.
 */
static void fghReshapeWindowByHandle ( SFG_WindowHandleType handle,
                                       int width, int height )
{
    SFG_Window *current_window = fgStructure.Window;

    SFG_Window* window = fgWindowByHandle( handle );
    glutes_return_if_fail( window != NULL );


#if TARGET_HOST_UNIX_X11

    XResizeWindow( fgDisplay.Display, window->Window.Handle,
                   width, height );
    XFlush( fgDisplay.Display ); /* XXX Shouldn't need this */

#elif TARGET_HOST_WIN32

    {
        RECT rect;

        /*
         * For windowed mode, get the current position of the
         * window and resize taking the size of the frame
         * decorations into account.
         */

        GetWindowRect( window->Window.Handle, &rect );
        rect.right  = rect.left + width;
        rect.bottom = rect.top  + height;

        if ( window->Parent == NULL )
        {
            if ( ! window->IsMenu && !window->State.IsGameMode )
            {
#ifndef _WIN32_WCE
                rect.right  += GetSystemMetrics( SM_CXSIZEFRAME ) * 2;
                rect.bottom += GetSystemMetrics( SM_CYSIZEFRAME ) * 2 +
                               GetSystemMetrics( SM_CYCAPTION );
#endif
            }
        }
        else
        {
            GetWindowRect( window->Parent->Window.Handle, &rect );
#ifdef _WIN32_WCE
			GetClientRect(window->Parent->Window.Handle, &rect);
			//AdjustWindowRectEx ( &rect, WS_CLIPSIBLINGS |
		//							  WS_CLIPCHILDREN, FALSE, 0 );
#else
            AdjustWindowRect ( &rect, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS |
                                      WS_CLIPCHILDREN, FALSE );			
#endif
        }

#ifndef _WIN32_WCE
        /*
         * SWP_NOACTIVATE	Do not activate the window
         * SWP_NOOWNERZORDER	Do not change position in z-order
         * SWP_NOSENDCHANGING	Supress WM_WINDOWPOSCHANGING message
         * SWP_NOZORDER		Retains the current Z order (ignore 2nd param)
         */

        SetWindowPos( window->Window.Handle,
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
#endif
    }

    /*
     * XXX Should update {window->State.OldWidth, window->State.OldHeight}
     * XXX to keep in lockstep with UNIX_X11 code.
     */
    if( FETCH_WCB( *window, Reshape ) )
        INVOKE_WCB( *window, Reshape, ( width, height ) );
    else
    {
        fgSetWindow( window );
        glViewport( 0, 0, width, height );
    }

#endif

    /*
     * Force a window redraw.  In Windows at least this is only a partial
     * solution:  if the window is increasing in size in either dimension,
     * the already-drawn part does not get drawn again and things look funny.
     * But without this we get this bad behaviour whenever we resize the
     * window.
     */
    window->State.Redisplay = GL_TRUE;

    if( window->IsMenu )
        fgSetWindow( current_window );
}

/*
 * Calls a window's redraw method. This is used when
 * a redraw is forced by the incoming window messages.
 */
static void fghRedrawWindowByHandle ( SFG_WindowHandleType handle )
{
    SFG_Window* window = fgWindowByHandle( handle );
    glutes_return_if_fail( window );
    glutes_return_if_fail( FETCH_WCB ( *window, Display ) );

    window->State.Redisplay = GL_FALSE;

    glutes_return_if_fail( window->State.Visible );

    if( window->State.NeedToResize )
    {
        SFG_Window *current_window = fgStructure.Window;

        fgSetWindow( window );

        fghReshapeWindowByHandle( 
            window->Window.Handle,
            window->State.Width,
            window->State.Height
        );

        window->State.NeedToResize = GL_FALSE;
        fgSetWindow ( current_window );
    }

    INVOKE_WCB( *window, Display, ( ) );
}

/*
 * A static helper function to execute display callback for a window
 */
static void fghcbDisplayWindow( SFG_Window *window,
                                SFG_Enumerator *enumerator )
{
    if( window->State.Redisplay &&
        window->State.Visible )
    {
        /*
         * XXX Resizing should *not* depend upon whether there
         * XXX is a pending redisplay flag, as far as I can tell.
         * XXX
         * XXX Note, too, that the {NeedToResize} flag is a little
         * XXX fuzzy in its meaning, since for WIN32, this also
         * XXX means "we need to tell the application that the window has
         * XXX changed size", while in X11, it only means "we need
         * XXX to ask the window system to resize the window.
         * XXX Splitting the flag's meaning might be desirable, but
         * XXX that could complicate the code more.  (On X11, the
         * XXX user callback is called as soon as the event is
         * XXX discovered, but resizing the window is postponed
         * XXX until after other events.)
         */
        if( window->State.NeedToResize )
        {
            SFG_Window *current_window = fgStructure.Window;

            fgSetWindow( window );

            fghReshapeWindowByHandle( 
                window->Window.Handle,
                window->State.Width,
                window->State.Height
            );

            window->State.NeedToResize = GL_FALSE;
            fgSetWindow ( current_window );
        }

        window->State.Redisplay = GL_FALSE;

#if TARGET_HOST_UNIX_X11
        {
            SFG_Window *current_window = fgStructure.Window;

            INVOKE_WCB( *window, Display, ( ) );
            fgSetWindow( current_window );
        }
#elif TARGET_HOST_WIN32
        RedrawWindow(
            window->Window.Handle, NULL, NULL, 
            RDW_NOERASE | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW
        );
#endif
    }

    fgEnumSubWindows( window, fghcbDisplayWindow, enumerator );
}

/*
 * Make all windows perform a display call
 */
static void fghDisplayAll( void )
{
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data  =  NULL;

    fgEnumWindows( fghcbDisplayWindow, &enumerator );
}

/*
 * Window enumerator callback to check for the joystick polling code
 */
static void fghcbCheckJoystickPolls( SFG_Window *window,
                                     SFG_Enumerator *enumerator )
{
#ifndef _WIN32_WCE
    long int checkTime = fgElapsedTime( );

    if( window->State.JoystickLastPoll + window->State.JoystickPollRate <=
        checkTime )
    {
        fgJoystickPollWindow( window );
        window->State.JoystickLastPoll = checkTime;
    }

    fgEnumSubWindows( window, fghcbCheckJoystickPolls, enumerator );
#endif
}

/*
 * Check all windows for joystick polling
 */
static void fghCheckJoystickPolls( void )
{
#ifndef _WIN32_WCE
    SFG_Enumerator enumerator;

    enumerator.found = GL_FALSE;
    enumerator.data  =  NULL;

    fgEnumWindows( fghcbCheckJoystickPolls, &enumerator );
#endif
}

/*
 * Check the global timers
 */
static void fghCheckTimers( void )
{
    long checkTime = fgElapsedTime( );
    SFG_Timer *timer;

    while( timer = fgState.Timers.First )
    {
        if( timer->TriggerTime > checkTime )
            break;

        fgListRemove( &fgState.Timers, &timer->Node );
        fgListAppend( &fgState.FreeTimers, &timer->Node );

        timer->Callback( timer->ID );
    }
}

/*
 * Elapsed Time
 */
long fgElapsedTime( void )
{
    if ( fgState.Time.Set )
    {
#if TARGET_HOST_UNIX_X11
        struct timeval now;
        long elapsed;

        gettimeofday( &now, NULL );

        elapsed = (now.tv_usec - fgState.Time.Value.tv_usec) / 1000;
        elapsed += (now.tv_sec - fgState.Time.Value.tv_sec) * 1000;

        return elapsed;
#elif TARGET_HOST_WIN32
        return GetTickCount() - fgState.Time.Value;
#endif
    }
    else
    {
#if TARGET_HOST_UNIX_X11
        gettimeofday( &fgState.Time.Value, NULL );
#elif TARGET_HOST_WIN32
        fgState.Time.Value = GetTickCount();
#endif
        fgState.Time.Set = GL_TRUE ;

        return 0 ;
    }
}

/*
 * Error Messages.
 */
void fgError( const char *fmt, ... )
{
	char text[1024];
#ifdef UNICODE
	WCHAR wtext[1024];
#endif
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "glutes ");
    if( fgState.ProgramName )
        fprintf (stderr, "(%s): ", fgState.ProgramName);
	vsprintf(text, fmt, ap );
	fprintf(stderr, text);
    fprintf( stderr, "\n" );

    va_end( ap );
#ifdef WIN32
#	ifdef UNICODE
		wsprintf(wtext, L"%S", text);
		MessageBox(0, wtext, TEXT("GLUT|ES Error"), MB_ICONERROR|MB_SETFOREGROUND|MB_TOPMOST);
#	else
		MessageBox(0, text, TEXT("GLUT|ES Error"), MB_ICONERROR);
#	endif
#endif

    if ( fgState.Initialised )
        fgDeinitialize ();

    exit( 1 );
}

void fgWarning( const char *fmt, ... )
{
	char text[1024];
#ifdef UNICODE
	WCHAR wtext[1024];
#endif
    va_list ap;

    va_start( ap, fmt );

    fprintf( stderr, "glutes ");
    if( fgState.ProgramName )
        fprintf( stderr, "(%s): ", fgState.ProgramName );
	vsprintf(text, fmt, ap );
	fprintf(stderr, text);
    fprintf( stderr, "\n" );

    va_end( ap );

#ifdef WIN32
#	ifdef UNICODE
		wsprintf(wtext, L"%S", text);
		MessageBox(0, wtext, TEXT("Warning"), MB_ICONERROR);
#	else
		MessageBox(0, text, TEXT("Warning"), MB_ICONERROR);
#	endif
#endif
}

/*
 * Indicates whether Joystick events are being used by ANY window.
 *
 * The current mechanism is to walk all of the windows and ask if
 * there is a joystick callback.  Certainly in some cases, maybe
 * in all cases, the joystick is attached to the system and accessed
 * from ONE point by GLUT/freeglut, so this is not the right way,
 * in general, to do this.  However, the Joystick code is segregated
 * in its own little world, so we can't access the information that
 * we need in order to do that nicely.
 *
 * Some alternatives:
 *  * Store Joystick data into freeglut global state.
 *  * Provide NON-static functions or data from Joystick *.c file.
 *
 * Basically, the RIGHT way to do this requires knowing something
 * about the Joystick.  Right now, the Joystick code is behind
 * an opaque wall.
 *
 */
static void fgCheckJoystickCallback( SFG_Window* w, SFG_Enumerator* e)
{
    if( FETCH_WCB( *w, Joystick ) )
    {
        e->found = GL_TRUE;
        e->data = w;
    }
    fgEnumSubWindows( w, fgCheckJoystickCallback, e );
}
static int fgHaveJoystick( void )
{
    SFG_Enumerator enumerator;
    enumerator.found = GL_FALSE;
    enumerator.data = NULL;
    fgEnumWindows( fgCheckJoystickCallback, &enumerator );
    return !!enumerator.data;
}
static void fgHavePendingRedisplaysCallback( SFG_Window* w, SFG_Enumerator* e)
{
    if( w->State.Redisplay )
    {
        e->found = GL_TRUE;
        e->data = w;
    }
    fgEnumSubWindows( w, fgHavePendingRedisplaysCallback, e );
}        
static int fgHavePendingRedisplays (void)
{
    SFG_Enumerator enumerator;
    enumerator.found = GL_FALSE;
    enumerator.data = NULL;
    fgEnumWindows( fgHavePendingRedisplaysCallback, &enumerator );
    return !!enumerator.data;
}
/*
 * Returns the number of GLUT ticks (milliseconds) till the next timer event.
 */
static long fgNextTimer( void )
{
    long ret = INT_MAX;
    SFG_Timer *timer = fgState.Timers.First;

    if( timer )
        ret = timer->TriggerTime - fgElapsedTime();
    if( ret < 0 )
        ret = 0;

    return ret;
}
/*
 * Does the magic required to relinquish the CPU until something interesting
 * happens.
 */
static void fgSleepForEvents( void )
{
#if TARGET_HOST_UNIX_X11
    fd_set fdset;
    int err;
    int socket;
    struct timeval wait;
    long msec;    
    
    if( fgState.IdleCallback || fgHavePendingRedisplays( ) )
        return;
    socket = ConnectionNumber( fgDisplay.Display );
    FD_ZERO( &fdset );
    FD_SET( socket, &fdset );
    
    msec = fgNextTimer( );
    if( fgHaveJoystick( ) )
        msec = MIN( msec, 10 );

    wait.tv_sec = msec / 1000;
    wait.tv_usec = (msec % 1000) * 1000;
    err = select( socket+1, &fdset, NULL, NULL, &wait );

    if( -1 == err )
        fgWarning ( "glutes select() error: %d\n", errno );
    
#elif TARGET_HOST_WIN32
#endif
}

#if TARGET_HOST_UNIX_X11
/*
 * Returns GLUT modifier mask for an XEvent.
 */
int fgGetXModifiers( XEvent *event )
{
    int ret = 0;

    if( event->xkey.state & ( ShiftMask | LockMask ) )
        ret |= GLUT_ACTIVE_SHIFT;
    if( event->xkey.state & ControlMask )
        ret |= GLUT_ACTIVE_CTRL;
    if( event->xkey.state & Mod1Mask )
        ret |= GLUT_ACTIVE_ALT;
    
    return ret;
}
#endif


/* -- INTERFACE FUNCTIONS -------------------------------------------------- */

/*
 * Executes a single iteration in the freeglut processing loop.
 */
void FGAPIENTRY glutMainLoopEvent( void )
{
#if TARGET_HOST_UNIX_X11
    SFG_Window* window;
    XEvent event;

    /*
     * This code was repeated constantly, so here it goes into a definition:
     */
#define GETWINDOW(a)                             \
    window = fgWindowByHandle( event.a.window ); \
    if( window == NULL )                         \
        break;

#define GETMOUSE(a)                              \
    window->State.MouseX = event.a.x;            \
    window->State.MouseY = event.a.y;

    glutes_assert_ready;

    while( XPending( fgDisplay.Display ) )
    {
        XNextEvent( fgDisplay.Display, &event );

        switch( event.type )
        {
        case ClientMessage:
            /*
             * Destroy the window when the WM_DELETE_WINDOW message arrives
             */
            if( (Atom) event.xclient.data.l[ 0 ] == fgDisplay.DeleteWindow )
            {
                GETWINDOW( xclient ); 

                fgDestroyWindow ( window );

                if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
                {
                    fgDeinitialize( );
                    exit( 0 );
                }

                fgState.ExecState = GLUT_EXEC_STATE_STOP;
                return;
            }
            break;

            /*
             * CreateNotify causes a configure-event so that sub-windows are
             * handled compatibly with GLUT.  Otherwise, your sub-windows
             * (in freeglut only) will not get an initial reshape event,
             * which can break things.
             *
             * GLUT presumably does this because it generally tries to treat
             * sub-windows the same as windows.
             *
             * XXX Technically, GETWINDOW( xconfigure ) and
             * XXX {event.xconfigure} may not be legit ways to get at
             * XXX data for CreateNotify events.  In practice, the data
             * XXX is in a union which is laid out much the same either
             * XXX way.  But if you want to split hairs, this isn't legit,
             * XXX and we should instead duplicate some code.
             */
        case CreateNotify:
        case ConfigureNotify:
            GETWINDOW( xconfigure );
            {
                int width = event.xconfigure.width;
                int height = event.xconfigure.height;

                if( ( width != window->State.OldWidth ) ||
                    ( height != window->State.OldHeight ) )
                {
                    window->State.OldWidth = width;
                    window->State.OldHeight = height;
                    if( FETCH_WCB( *window, Reshape ) )
                        INVOKE_WCB( *window, Reshape, ( width, height ) );
                    else
                    {
                        fgSetWindow( window );
                        glViewport( 0, 0, width, height );
                    }
                    glutPostRedisplay( );
                }
            }
            break;

        case DestroyNotify:
            /*
             * This is sent to confirm the XDestroyWindow call.
             * XXX WHY is this commented out?  Should we re-enable it?
             */
            /* fgAddToWindowDestroyList ( window ); */
            break;

        case Expose:
            /*
             * We are too dumb to process partial exposes...
             *
             * XXX Well, we could do it.  However, it seems to only
             * XXX be potentially useful for single-buffered (since
             * XXX double-buffered does not respect viewport when we
             * XXX do a buffer-swap).
             *
             * XXX GETWINDOW( xexpose );
             * XXX fgSetWindow( window );
             * XXX glutPostRedisplay( );
             */
            if( event.xexpose.count == 0 )
                fghRedrawWindowByHandle( event.xexpose.window );
            break;

        case MapNotify:
        case UnmapNotify:
            /*
             * If we never do anything with this, can we just not ask to
             * get these messages?
             */
            break;

        case MappingNotify:
            /*
             * Have the client's keyboard knowledge updated (xlib.ps,
             * page 206, says that's a good thing to do)
             */
            XRefreshKeyboardMapping( (XMappingEvent *) &event );
            break;

        case VisibilityNotify:
        {
            GETWINDOW( xvisibility ); 
            /*
             * XXX INVOKE_WCB() does this check for us.
             */
            if( ! FETCH_WCB( *window, WindowStatus ) )
                break;
            fgSetWindow( window );

            /*
             * Sending this event, the X server can notify us that the window
             * has just acquired one of the three possible visibility states:
             * VisibilityUnobscured, VisibilityPartiallyObscured or
             * VisibilityFullyObscured
             */
            switch( event.xvisibility.state )
            {
            case VisibilityUnobscured:
                INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;
                
            case VisibilityPartiallyObscured:
                INVOKE_WCB( *window, WindowStatus,
                            ( GLUT_PARTIALLY_RETAINED ) );
                window->State.Visible = GL_TRUE;
                break;
                
            case VisibilityFullyObscured:
                INVOKE_WCB( *window, WindowStatus, ( GLUT_FULLY_COVERED ) );
                window->State.Visible = GL_FALSE;
                break;

            default:
                fgWarning( "Uknown X visibility state: %d",
                           event.xvisibility.state );
                break;
            }
        }
        break;

        case EnterNotify:
        case LeaveNotify:
            GETWINDOW( xcrossing );
            GETMOUSE( xcrossing );
            INVOKE_WCB( *window, Entry, ( ( EnterNotify == event.type ) ?
                                          GLUT_ENTERED :
                                          GLUT_LEFT ) );
            break;

        case MotionNotify:
        {
            GETWINDOW( xmotion );
            GETMOUSE( xmotion );

#ifdef USE_GLMENU
            if( window->ActiveMenu )
            {
                if( window == window->ActiveMenu->ParentWindow )
                {
                    window->ActiveMenu->Window->State.MouseX =
                        event.xmotion.x_root - window->ActiveMenu->X;
                    window->ActiveMenu->Window->State.MouseY =
                        event.xmotion.y_root - window->ActiveMenu->Y;
                }
                window->ActiveMenu->Window->State.Redisplay = GL_TRUE ;
                fgSetWindow( window->ActiveMenu->ParentWindow );

                break;
            }
#endif

            /*
             * XXX For more than 5 buttons, just check {event.xmotion.state},
             * XXX rather than a host of bit-masks?  Or maybe we need to
             * XXX track ButtonPress/ButtonRelease events in our own
             * XXX bit-mask?
             */
#define BUTTON_MASK \
  ( Button1Mask | Button2Mask | Button3Mask | Button4Mask | Button5Mask )
            if ( event.xmotion.state & BUTTON_MASK )
                INVOKE_WCB( *window, Motion, ( event.xmotion.x,
                                               event.xmotion.y ) );
            else
                INVOKE_WCB( *window, Passive, ( event.xmotion.x,
                                                event.xmotion.y ) );
        }
        break;

        case ButtonRelease:
        case ButtonPress:
        {
            GLboolean pressed = GL_TRUE;
            int button;

            if( event.type == ButtonRelease )
                pressed = GL_FALSE ;

            /*
             * A mouse button has been pressed or released. Traditionally,
             * break if the window was found within the freeglut structures.
             */
            GETWINDOW( xbutton );
            GETMOUSE( xbutton );
          
            /*
             * An X button (at least in XFree86) is numbered from 1.
             * A GLUT button is numbered from 0.
             * Old GLUT passed through buttons other than just the first
             * three, though it only gave symbolic names and official
             * support to the first three.
             */
            button = event.xbutton.button - 1;

            /*
             * XXX This comment is replicated in the WIN32 section and
             * XXX maybe also in the menu code.  Can we move the info
             * XXX to one central place and *reference* it from here?
             *
             * Do not execute the application's mouse callback if a menu
             * is hooked to this button.  In that case an appropriate
             * private call should be generated.
             * Near as I can tell, this is the menu behaviour:
             *  - Down-click the menu button, menu not active:  activate
             *    the menu with its upper left-hand corner at the mouse
             *    location.
             *  - Down-click any button outside the menu, menu active:
             *    deactivate the menu
             *  - Down-click any button inside the menu, menu active:
             *    select the menu entry and deactivate the menu
             *  - Up-click the menu button, menu not active:  nothing happens
             *  - Up-click the menu button outside the menu, menu active:
             *    nothing happens
             *  - Up-click the menu button inside the menu, menu active:
             *    select the menu entry and deactivate the menu
             */
            /* Window has an active menu, it absorbs any mouse click */
#ifdef USE_GLMENU
            if( window->ActiveMenu )
            {
                if( window == window->ActiveMenu->ParentWindow )
                {
                    window->ActiveMenu->Window->State.MouseX =
                        event.xbutton.x_root - window->ActiveMenu->X;
                    window->ActiveMenu->Window->State.MouseY =
                        event.xbutton.y_root - window->ActiveMenu->Y;
                }
                
                /* In the menu, invoke the callback and deactivate the menu*/
                if( fgCheckActiveMenu( window->ActiveMenu->Window,
                                       window->ActiveMenu ) )
                {
                    /*
                     * Save the current window and menu and set the current
                     * window to the window whose menu this is
                     */
                    SFG_Window *save_window = fgStructure.Window;
                    SFG_Menu *save_menu = fgStructure.Menu;
                    SFG_Window *parent_window =
                        window->ActiveMenu->ParentWindow;
                    fgSetWindow( parent_window );
                    fgStructure.Menu = window->ActiveMenu;

                    /* Execute the menu callback */
                    fgExecuteMenuCallback( window->ActiveMenu );
                    fgDeactivateMenu( parent_window );

                    /* Restore the current window and menu */
                    fgSetWindow( save_window );
                    fgStructure.Menu = save_menu;
                }
                else if( pressed )
                    /*
                     * Outside the menu, deactivate if it's a downclick
                     *
                     * XXX This isn't enough.  A downclick outside of
                     * XXX the interior of our freeglut windows should also
                     * XXX deactivate the menu.  This is more complicated.
                     */
                    fgDeactivateMenu( window->ActiveMenu->ParentWindow );

                /*
                 * XXX Why does an active menu require a redisplay at
                 * XXX this point?  If this can come out cleanly, then
                 * XXX it probably should do so; if not, a comment should
                 * XXX explain it.
                 */
                window->State.Redisplay = GL_TRUE;
                break;
            }
#endif /* USE_GLMENU */
            /*
             * No active menu, let's check whether we need to activate one.
             */
            if( ( 0 <= button ) &&
                ( FREEGLUT_MAX_MENUS > button ) &&
                ( window->Menu[ button ] ) &&
                pressed )
            {
                /*
                 * XXX Posting a requisite Redisplay seems bogus.
                 */
                window->State.Redisplay = GL_TRUE;
                fgSetWindow( window );
                fgActivateMenu( window, button );
                break;
            }

            /*
             * Check if there is a mouse or mouse wheel callback hooked to the
             * window
             */
            if( ! FETCH_WCB( *window, Mouse ) &&
                ! FETCH_WCB( *window, MouseWheel ) )
                break;

            fgState.Modifiers = fgGetXModifiers( &event );

            /*
             * Finally execute the mouse or mouse wheel callback
             *
             * XXX Use a symbolic constant, *not* "4"!  ("3, sire!")
             */
            if( ( button < 3 ) || ( ! FETCH_WCB( *window, MouseWheel ) ) )
                INVOKE_WCB( *window, Mouse, ( button,
                                              pressed ? GLUT_DOWN : GLUT_UP,
                                              event.xbutton.x,
                                              event.xbutton.y )
                );
            else
            {
                /*
                 * Map 4 and 5 to wheel zero; EVEN to +1, ODD to -1
                 *  "  6 and 7 "    "   one; ...
                 *
                 * XXX This *should* be behind some variables/macros,
                 * XXX since the order and numbering isn't certain
                 * XXX See XFree86 configuration docs (even back in the
                 * XXX 3.x days, and especially with 4.x).
                 *
                 * XXX Note that {button} has already been decremeted
                 * XXX in mapping from X button numbering to GLUT.
                 */
                int wheel_number = (button - 3) / 2;
                int direction = -1;
                if( button % 2 )
                    direction = 1;
                
                if( pressed )
                    INVOKE_WCB( *window, MouseWheel, ( wheel_number,
                                                       direction,
                                                       event.xbutton.x,
                                                       event.xbutton.y )
                    );
            }

            /*
             * Trash the modifiers state
             */
            fgState.Modifiers = 0xffffffff;
        }
        break;

        case KeyRelease:
        case KeyPress:
        {
            FGCBKeyboard keyboard_cb;
            FGCBSpecial special_cb;

            GETWINDOW( xkey );
            GETMOUSE( xkey );

            if( event.type == KeyPress )
            {
                keyboard_cb = FETCH_WCB( *window, Keyboard );
                special_cb  = FETCH_WCB( *window, Special  );
            }
            else
            {
                keyboard_cb = FETCH_WCB( *window, KeyboardUp );
                special_cb  = FETCH_WCB( *window, SpecialUp  );
            }

            /*
             * Is there a keyboard/special callback hooked for this window?
             */
            if( keyboard_cb || special_cb )
            {
                XComposeStatus composeStatus;
                char asciiCode[ 32 ];
                KeySym keySym;
                int len;

                /*
                 * Check for the ASCII/KeySym codes associated with the event:
                 */
                len = XLookupString( &event.xkey, asciiCode, sizeof(asciiCode),
                                     &keySym, &composeStatus
                );

                /*
                 * GLUT API tells us to have two separate callbacks...
                 */
                if( len > 0 )
                {
                    /*
                     * ...one for the ASCII translateable keypresses...
                     */
                    if( keyboard_cb )
                    {
                        fgSetWindow( window );
                        fgState.Modifiers = fgGetXModifiers( &event );
                        keyboard_cb( asciiCode[ 0 ],
                                     event.xkey.x, event.xkey.y
                        );
                        fgState.Modifiers = 0xffffffff;
                    }
                }
                else
                {
                    int special = -1;

                    /*
                     * ...and one for all the others, which need to be
                     * translated to GLUT_KEY_Xs...
                     */
                    switch( keySym )
                    {
                    case XK_F1:     special = GLUT_KEY_F1;     break;
                    case XK_F2:     special = GLUT_KEY_F2;     break;
                    case XK_F3:     special = GLUT_KEY_F3;     break;
                    case XK_F4:     special = GLUT_KEY_F4;     break;
                    case XK_F5:     special = GLUT_KEY_F5;     break;
                    case XK_F6:     special = GLUT_KEY_F6;     break;
                    case XK_F7:     special = GLUT_KEY_F7;     break;
                    case XK_F8:     special = GLUT_KEY_F8;     break;
                    case XK_F9:     special = GLUT_KEY_F9;     break;
                    case XK_F10:    special = GLUT_KEY_F10;    break;
                    case XK_F11:    special = GLUT_KEY_F11;    break;
                    case XK_F12:    special = GLUT_KEY_F12;    break;

                    case XK_Left:   special = GLUT_KEY_LEFT;   break;
                    case XK_Right:  special = GLUT_KEY_RIGHT;  break;
                    case XK_Up:     special = GLUT_KEY_UP;     break;
                    case XK_Down:   special = GLUT_KEY_DOWN;   break;

                    case XK_KP_Prior:
                    case XK_Prior:  special = GLUT_KEY_PAGE_UP; break;
                    case XK_KP_Next:
                    case XK_Next:   special = GLUT_KEY_PAGE_DOWN; break;
                    case XK_KP_Home:
                    case XK_Home:   special = GLUT_KEY_HOME;   break;
                    case XK_KP_End:
                    case XK_End:    special = GLUT_KEY_END;    break;
                    case XK_KP_Insert:
                    case XK_Insert: special = GLUT_KEY_INSERT; break;
                    }

                    /*
                     * Execute the callback (if one has been specified),
                     * given that the special code seems to be valid...
                     */
                    if( special_cb && (special != -1) )
                    {
                        fgSetWindow( window );
                        fgState.Modifiers = fgGetXModifiers( &event );
                        special_cb( special, event.xkey.x, event.xkey.y );
                        fgState.Modifiers = 0xffffffff;
                    }
                }
            }
        }
        break;

        case ReparentNotify:
            break; /* XXX Should disable this event */

        default:
            fgWarning ("Unknown X event type: %d", event.type);
            break;
        }
    }

#elif TARGET_HOST_WIN32

    MSG stMsg;
	HACCEL hAccelTable = LoadAccelerators(GetModuleHandle(NULL), NULL);

    while( PeekMessage( &stMsg, NULL, 0, 0, PM_NOREMOVE ) )
    {
        if( GetMessage( &stMsg, NULL, 0, 0 ) == 0 )
        {
            if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
            {
                fgDeinitialize( );
                exit( 0 );
            }
            fgState.ExecState = GLUT_EXEC_STATE_STOP;
            return;
        }
		if(!TranslateAccelerator(stMsg.hwnd, hAccelTable, &stMsg))
        {
			TranslateMessage( &stMsg );
			DispatchMessage( &stMsg );
		}
    }
#endif

    if( fgState.Timers.First )
        fghCheckTimers( );
    fghCheckJoystickPolls( );
    fghDisplayAll( );

    fgCloseWindows( );
}

/*
 * Enters the freeglut processing loop.
 * Stays until the "ExecState" changes to "GLUT_EXEC_STATE_STOP".
 */
void FGAPIENTRY glutMainLoop( void )
{
#if TARGET_HOST_WIN32
    SFG_Window *window = (SFG_Window *)fgStructure.Windows.First ;
#endif

    glutes_assert_ready;

#if TARGET_HOST_WIN32
    /*
     * Processing before the main loop:  If there is a window which is open and
     * which has a visibility callback, call it.  I know this is an ugly hack,
     * but I'm not sure what else to do about it.  Ideally we should leave
     * something uninitialized in the create window code and initialize it in
     * the main loop, and have that initialization create a "WM_ACTIVATE"
     * message.  Then we would put the visibility callback code in the
     * "case WM_ACTIVATE" block below.         - John Fay -- 10/24/02
     */
    while( window )
    {
        if ( FETCH_WCB( *window, Visibility ) )
        {
            SFG_Window *current_window = fgStructure.Window ;

            INVOKE_WCB( *window, Visibility, ( window->State.Visible ) );
            fgSetWindow( current_window );
        }
        
        window = (SFG_Window *)window->Node.Next ;
    }
#endif 

    fgState.ExecState = GLUT_EXEC_STATE_RUNNING ;
    while( fgState.ExecState == GLUT_EXEC_STATE_RUNNING )
    {
        SFG_Window *window;

        glutMainLoopEvent( );
        /*
         * Step through the list of windows, seeing if there are any
         * that are not menus
         */ 
        for( window = ( SFG_Window * )fgStructure.Windows.First;
             window;
             window = ( SFG_Window * )window->Node.Next )
            if ( ! ( window->IsMenu ) )
                break;
        
        if( ! window )
            fgState.ExecState = GLUT_EXEC_STATE_STOP;
        else
        {
            if( fgState.IdleCallback )
                fgState.IdleCallback( );

            fgSleepForEvents( );
        }
    }

    /*
     * When this loop terminates, destroy the display, state and structure
     * of a freeglut session, so that another glutInit() call can happen
     */
    fgDeinitialize( );
    if( fgState.ActionOnWindowClose == GLUT_ACTION_EXIT )
        exit( 0 );
}

/*
 * Leaves the freeglut processing loop.
 */
void FGAPIENTRY glutLeaveMainLoop( void )
{
    fgState.ExecState = GLUT_EXEC_STATE_STOP ;
}

#if TARGET_HOST_WIN32
/*
 * Determine a GLUT modifer mask based on MS-WINDOWS system info.
 */
int fgGetWin32Modifiers (void)
{
    return
        ( ( ( GetKeyState( VK_LSHIFT   ) < 0 ) ||
            ( GetKeyState( VK_RSHIFT   ) < 0 )) ? GLUT_ACTIVE_SHIFT : 0 ) |
        ( ( ( GetKeyState( VK_LCONTROL ) < 0 ) ||
            ( GetKeyState( VK_RCONTROL ) < 0 )) ? GLUT_ACTIVE_CTRL  : 0 ) |
        ( ( ( GetKeyState( VK_LMENU    ) < 0 ) ||
            ( GetKeyState( VK_RMENU    ) < 0 )) ? GLUT_ACTIVE_ALT   : 0 );
}
#endif


#if TARGET_HOST_WIN32
/**! \brief   Create a DIB section bitmap.
 * \param   hDC DC handle.
 * \return  Returns the handle to the created bitmap.																   
 */
static HBITMAP createPixmap(HDC hDC, SFG_Window* w)
{
	const size_t    bmiSize = sizeof(BITMAPINFO) + 256U*sizeof(RGBQUAD);
	BITMAPINFO*     bmi;
	DWORD*          p;
	int SamplePixmapBits = 16;
	RECT rect;
	int width, height;
	GetClientRect(w->Window.Handle, &rect);
	width = rect.right - rect.left;
	height = rect.bottom - rect.top;

	bmi = (BITMAPINFO*)malloc(bmiSize);
	memset(bmi, 0, bmiSize);
	
	p = (DWORD*)bmi->bmiColors;

#ifdef _WIN32_WCE
	{
	if(fgDisplay.FrameBufferInfo.wFormat == FBI_FORMAT_555)
	{
		p[0] = 0x1F << 0;   /* r */
		p[1] = 0x1F << 5;   /* g */
		p[2] = 0x1F << 10;  /* b */
	}
	else if(fgDisplay.FrameBufferInfo.wFormat == FBI_FORMAT_565)
	{
		p[0] = 0xFF;        /* r */
		p[1] = 0xFF00;      /* g */
		p[2] = 0xFF0000;    /* b */
	}
	else 
	{
		SamplePixmapBits = 32;
		p[0] = 0xFF;        /* r */
		p[1] = 0xFF00;      /* g */
		p[2] = 0xFF0000;    /* b */
	}
	}
#else
	{
		SamplePixmapBits = 32;
		p[0] = 0xFF;        /* r */
		p[1] = 0xFF00;      /* g */
		p[2] = 0xFF0000;    /* b */
	}
#endif
	
	bmi->bmiHeader.biSize           = sizeof(BITMAPINFOHEADER);
#if defined (USE_GAPI)
	bmi->bmiHeader.biWidth          = GetSystemMetrics(SM_CXSCREEN),
	bmi->bmiHeader.biHeight			= -GetSystemMetrics(SM_CYSCREEN),
#else
	bmi->bmiHeader.biWidth			= width; 
	bmi->bmiHeader.biHeight         = height; 
#endif
	bmi->bmiHeader.biPlanes         = (short)1;
	bmi->bmiHeader.biBitCount       = (unsigned int)SamplePixmapBits;
	bmi->bmiHeader.biCompression    = BI_BITFIELDS;
	bmi->bmiHeader.biSizeImage      = 0;
	bmi->bmiHeader.biXPelsPerMeter  = 0;
	bmi->bmiHeader.biYPelsPerMeter  = 0;
	bmi->bmiHeader.biClrUsed        = 3;
	bmi->bmiHeader.biClrImportant   = 0;

	return CreateDIBSection(hDC, bmi, DIB_RGB_COLORS, (void**)&w->Window.bits, NULL, 0);
}

static void deletePixmap (HBITMAP bitmap)
{
	DeleteObject(bitmap);
}
#endif //TARGET_HOST_WIN32

EGLint fgEGLConfigAttribs[] = 
{
		EGL_SURFACE_TYPE,			EGL_WINDOW_BIT,
		EGL_BUFFER_SIZE,			0,
		EGL_RED_SIZE,				0,
		EGL_GREEN_SIZE,				0,
		EGL_BLUE_SIZE,				0,
		EGL_ALPHA_SIZE,				EGL_DONT_CARE,
//		EGL_CONFIG_CAVEAT,			EGL_DONT_CARE,
//		EGL_CONFIG_ID,				EGL_DONT_CARE,
		EGL_DEPTH_SIZE,				32,
//		EGL_LEVEL,					0,
//		EGL_NATIVE_RENDERABLE,		EGL_DONT_CARE,
//		EGL_NATIVE_VISUAL_STYLE,	EGL_DONT_CARE,
//		EGL_SAMPLE_BUFFERS,			0,
//		EGL_SAMPLES,				0,
		EGL_STENCIL_SIZE,			0,
//		EGL_TRANSPARENT_TYPE,		EGL_NONE,
//		EGL_TRANSPARENT_RED_VALUE,	EGL_DONT_CARE,
//		EGL_TRANSPARENT_GREEN_VALUE,EGL_DONT_CARE,
//		EGL_TRANSPARENT_BLUE_VALUE, EGL_DONT_CARE,
		EGL_NONE
};

void 
__glCreateContext(SFG_Window* w)
{
	//static beenHere = 0;
	EGLint numConfigs;

	if(w->Window.Context != EGL_NO_CONTEXT)
		return;

	w->Window.SurfaceType = EGL_WINDOW_BIT;

	fgEGLConfigAttribs[0 * 2 + 1] = w->Window.SurfaceType;

	if(fgEGLConfigAttribs[2 * 2 + 1] == 0)
	{
#ifdef _WIN32_WCE		
		if(fgDisplay.FrameBufferInfo.wFormat == FBI_FORMAT_555)
		{
			fgEGLConfigAttribs[1 * 2 + 1] = 
				fgEGLConfigAttribs[2 * 2 + 1] = 
				fgEGLConfigAttribs[3 * 2 + 1] = 5;
		}
		else if(fgDisplay.FrameBufferInfo.wFormat == FBI_FORMAT_565)
		{
			fgEGLConfigAttribs[1 * 2 + 1] = 5;
			fgEGLConfigAttribs[2 * 2 + 1] = 6;
			fgEGLConfigAttribs[3 * 2 + 1] = 5;
		}
		else
#endif
		{
			fgEGLConfigAttribs[1 * 2 + 1] = 
				fgEGLConfigAttribs[2 * 2 + 1] = 
				fgEGLConfigAttribs[3 * 2 + 1] = 8;
		}
	}
	
	if(!eglChooseConfig(fgDisplay.eglDisplay, fgEGLConfigAttribs, &w->Window.eglConfig, 1, &numConfigs))
		fgError("Unable to choose OpenGL|ES config\n(ErrCode: %d)!", eglGetError());
	
	w->Window.Context = eglCreateContext(fgDisplay.eglDisplay, w->Window.eglConfig, NULL, NULL);

	if(w->Window.Context == EGL_NO_CONTEXT)
		fgError("Unable to create OpenGL|ES context\n(ErrCode: %d)!", eglGetError());

	/*if( ! beenHere )
	{
		FILE *f = fopen("current_config.txt", "w");
		GLint r,g,b,a,depth,type;
		int j;		

		beenHere = 1;

		for(j=0;j<10*2+1;j+=2)
			fprintf(f,". %d = %d", fgEGLConfigAttribs[j], fgEGLConfigAttribs[j+1]);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_RED_SIZE,&r);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_GREEN_SIZE,&g);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_BLUE_SIZE,&b);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_ALPHA_SIZE,&a);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_DEPTH_SIZE,&depth);
		eglGetConfigAttrib(fgDisplay.eglDisplay,w->Window.eglConfig,EGL_SURFACE_TYPE,&type);
		fprintf(f, "\n\nCurrent EGL Config (internal ID: 0x%x)\n",(int)w->Window.eglConfig);
		fprintf(f, "Surface type available:\t");
		if (type & EGL_WINDOW_BIT) fprintf(f, "Window "); 
		if (type & EGL_PBUFFER_BIT) fprintf(f, "PBuffer "); 
		if (type & EGL_PIXMAP_BIT) fprintf(f, "Pixmap "); 
		fprintf(f, "\nR: %i\tG: %i\tB: %i\tA: %i\n",r,g,b,a);
		fprintf(f, "Depth Buffer %i bits\n",depth);
		fclose(f);
	}*/
}

static void
__glBindContext(SFG_Window* w) 
{
	if(!w)
		return;


	if(w->Window.Context == EGL_NO_CONTEXT)
		__glCreateContext(w);

	//if(w->Window.Surface == EGL_NO_SURFACE)
	//	__glCreateSurface(w);

	if(!eglMakeCurrent(fgDisplay.eglDisplay, 
		w->Window.Surface, w->Window.Surface, 
		w->Window.Surface != EGL_NO_SURFACE ? w->Window.Context : EGL_NO_CONTEXT))
	{
		fgError("Unable to bind OpenGL|ES surface!");
	}
}

void 
__glDestroySurface(SFG_Window* w)
{
	if(w && w->Window.Surface != EGL_NO_SURFACE)
	{
		eglSwapBuffers(fgDisplay.eglDisplay, w->Window.Surface);
		eglMakeCurrent(fgDisplay.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, w->Window.Context);
		//eglDestroyContext(fgDisplay.eglDisplay, w->Window.Context);
		eglDestroySurface(fgDisplay.eglDisplay, w->Window.Surface);
		//w->Window.Context = EGL_NO_CONTEXT;
		w->Window.Surface = EGL_NO_SURFACE;
	}
}
void 
__glCreateSurface(SFG_Window* w)
{
	static int n = 1;
	if(!w) 
		return;

	if(w->Window.Context == EGL_NO_CONTEXT)
		__glCreateContext(w);

	if(w->Window.Surface != EGL_NO_SURFACE)
		return;//__glDestroySurface(w);

	// Debug : print the available configs
	/*if(n==1)
	{

		int i;
		EGLint attrib_list[] =
		{

			EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
			EGL_NONE
		};
		FILE *f = fopen("surfaces.txt", "w");
		EGLint num_config = 0;
		EGLConfig eglAvailableConfigs[256];
		// n = 0;
		if (!eglChooseConfig(fgDisplay.eglDisplay, attrib_list, eglAvailableConfigs, sizeof(eglAvailableConfigs)/sizeof(eglAvailableConfigs[0]),&num_config) || num_config == 0)
		{ 
			// break;
		}

		for (i=0;i<num_config;i++)
		{
			GLint r,g,b,a,depth,type;
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_RED_SIZE,&r);
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_GREEN_SIZE,&g);
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_BLUE_SIZE,&b);
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_ALPHA_SIZE,&a);
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_DEPTH_SIZE,&depth);
			eglGetConfigAttrib(fgDisplay.eglDisplay,eglAvailableConfigs[i],EGL_SURFACE_TYPE,&type);
			fprintf(f, "\n\nEGL Config %i (internal ID: 0x%x)\n",i,(int)eglAvailableConfigs[i]);
			fprintf(f, "Surface type available:\t");
			if (type & EGL_WINDOW_BIT)		fprintf(f, "Window "); 
			if (type & EGL_PBUFFER_BIT)	fprintf(f, "PBuffer "); 
			if (type & EGL_PIXMAP_BIT)		fprintf(f, "Pixmap "); 
			fprintf(f, "\nR: %i\tG: %i\tB: %i\tA: %i\n",r,g,b,a);
			fprintf(f, "Depth Buffer %i bits\n",depth);
		} 
		fclose(f);
	}*/

	if(w->Window.SurfaceType == EGL_WINDOW_BIT)
	{
		if((w->Window.Surface = eglCreateWindowSurface(fgDisplay.eglDisplay, w->Window.eglConfig, w->Window.Handle, NULL)) == EGL_NO_SURFACE)
		{
			//	fgError("Unable to create OpenGL|ES window surface\n(ErrCode: %d)!", eglGetError());
		}
	}

	// Debug : print the current config
	if(w->Window.SurfaceType == EGL_PIXMAP_BIT)
	{
#if TARGET_HOST_WIN32
		HDC hdc = GetDC(w->Window.Handle);
		w->Window.pixmap = createPixmap(hdc, w);
		ReleaseDC(w->Window.Handle, hdc);
		if((w->Window.Surface = eglCreatePixmapSurface(fgDisplay.eglDisplay, w->Window.eglConfig, (NativePixmapType)w->Window.pixmap, NULL)) == EGL_NO_SURFACE)
			fgError("Unable to make current OpenGL|ES pixmap surface\n(ErrCode: %d)!", eglGetError());
#endif //TARGET_HOST_WIN32
	}
	
	if(w->Window.SurfaceType == EGL_PBUFFER_BIT)
	{
		EGLint s_pbufferAttribs[] =
		{
			EGL_WIDTH,  0,
			EGL_HEIGHT, 0,
			EGL_NONE
		};
		s_pbufferAttribs[1] = w->State.Width;
		s_pbufferAttribs[3] = w->State.Height;
		if((w->Window.Surface = eglCreatePbufferSurface(fgDisplay.eglDisplay, w->Window.eglConfig, s_pbufferAttribs)) == EGL_NO_SURFACE)
			fgError("Unable to make current OpenGL|ES pbuffer surface\n(ErrCode: %d)!", eglGetError());
	}

	__glBindContext(w);
}





////////////////////////////
#if TARGET_HOST_WIN32

#ifndef USE_GLMENU

static int fgOnButtonDn(HWND hwnd, int x, int y, int button)
{
	SFG_WinMenu* menu;
    SFG_Window* window = fgWindowByHandle( hwnd );
	if(!window)
		return 0;
	
	/* finish the menu if we get a button down message (user must have
	cancelled the menu). */
	if(fgMappedMenu) 
	{
		POINT point;
		
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);
		fgItemSelected = NULL;
		fgFinishMenu(window, point.x, point.y);
		return 1;
	}
	
	/* Set the capture so we can get mouse events outside the window. */
	SetCapture(hwnd);
	
	if(window)
	{
		menu = fgGetMenuByNum( window->Menu[ button ] );
		if ( menu ) 
		{
			POINT point;
			point.x = x; point.y = y;
			ClientToScreen(window->Window.Handle, &point);
			fgMenuButton = 
#ifdef _WIN32_WCE
				0;
#else
			button == GLUT_RIGHT_BUTTON ? TPM_RIGHTBUTTON :
			button == GLUT_LEFT_BUTTON  ? TPM_LEFTBUTTON :			0x0001;
#endif
			//__glDestroySurface(window);
			eglMakeCurrent(fgDisplay.eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);//window->Window.Context); 
			fgStartMenu(menu, window, point.x, point.y, x, y);
			eglMakeCurrent(fgDisplay.eglDisplay, window->Window.Surface, window->Window.Surface, window->Window.Context); 
			//__glCreateSurface(window);
			return 1;
		} 
	}
	return 0;
}

static int fgOnButtonUp(HWND hwnd, int x, int y, int button)
{
    SFG_Window* window = fgWindowByHandle( hwnd );
	if(!window)
		return 0;

	/* Bail out if we're processing a menu. */
	if (fgMappedMenu) 
	{
		POINT point;
		GetCursorPos(&point);
		ScreenToClient(hwnd, &point);
		fgItemSelected = NULL;
		fgFinishMenu(window, point.x, point.y);
		return 1;
	}
	
	/* Release the mouse capture. */
	ReleaseCapture();
	
	return 0;
}


static void fgOnCommand( HWND hwnd, int id, HWND hwndCtl, UINT codeNotify )
{
	SFG_Window* window = fgWindowByHandle( hwnd );
	if ( fgMappedMenu ) 
	{
		POINT point;
#if 0
		if ( GetSubMenu( __glutHMenu, id ) )
			fgItemSelected = NULL;
		else
#endif
			fgItemSelected = fgGetUniqueMenuItem( fgMappedMenu, id );
		
		GetCursorPos( &point );
		ScreenToClient( hwnd, &point );
		
		fgFinishMenu( window, point.x, point.y );
	} 
}

#endif // USE_GLMENU

///////////////////////////////////////////////////////////////////////////////
// Register special keys (Calendar, Taks, Home & Programs) and send events to hWnd
void RegisterHotKeys(HWND hWnd)
{
#ifdef _WIN32_WCE
	RegisterHotKey(hWnd, 0xC1, MOD_WIN | MOD_KEYUP, 0xC1);
	RegisterHotKey(hWnd, 0xC2, MOD_WIN | MOD_KEYUP, 0xC2);
	RegisterHotKey(hWnd, 0xC3, MOD_WIN | MOD_KEYUP, 0xC3);
	RegisterHotKey(hWnd, 0xC4, MOD_WIN | MOD_KEYUP, 0xC4);
#endif //_WIN32_WCE
}

/*
 * The window procedure for handling Win32 events
 */
LRESULT CALLBACK fgWindowProc( HWND hWnd, UINT uMsg, WPARAM wParam,
                               LPARAM lParam )
{
    SFG_Window* window = fgWindowByHandle( hWnd );
    PAINTSTRUCT ps;
    LONG lRet = 1;
#ifdef _WIN32_WCE
	static SHACTIVATEINFO s_sai;
#endif

    if ( ( window == NULL ) && ( uMsg != WM_CREATE ) )
      return DefWindowProc( hWnd, uMsg, wParam, lParam );

	if(!fgMappedMenu)
		__glBindContext(window);

    /* printf ( "Window %3d message <%04x> %12d %12d\n", window?window->ID:0,
             uMsg, wParam, lParam ); */
    switch( uMsg )
    {
    case WM_CREATE:
        /*
         * The window structure is passed as the creation structure paramter...
         */
#ifdef _WIN32_WCE
		memset(&s_sai, 0, sizeof (s_sai));
		s_sai.cbSize = sizeof (s_sai);	
#endif
        window = (SFG_Window *) (((LPCREATESTRUCT) lParam)->lpCreateParams);
        assert( window != NULL );

        window->Window.Handle = hWnd;
        window->Window.Device = GetDC( hWnd ); 
#if 0
        if( window->IsMenu )
        {
            unsigned int current_DisplayMode = fgState.DisplayMode;
            fgState.DisplayMode = GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH;
            fgSetupPixelFormat( window, GL_FALSE, PFD_MAIN_PLANE );
            fgState.DisplayMode = current_DisplayMode;

            if( fgStructure.MenuContext )
                wglMakeCurrent( window->Window.Device,
                                fgStructure.MenuContext->Context
                );
            else
            {
                fgStructure.MenuContext =
                    (SFG_MenuContext *)malloc( sizeof(SFG_MenuContext) );
                fgStructure.MenuContext->Context =
                    wglCreateContext( window->Window.Device );
            }

            /* window->Window.Context = wglGetCurrentContext ();   */
            window->Window.Context = wglCreateContext( window->Window.Device );
		}
		else
#endif
		{
		}		
		/*window->State.NeedToResize = GL_TRUE;
		window->State.Width  = fgState.Size.X;
		window->State.Height = fgState.Size.Y;*/
		ReleaseDC( window->Window.Handle, window->Window.Device );
		break;

	case WM_INITMENUPOPUP:
		return 0;

    case WM_SIZE:
		if(GetActiveWindow() != hWnd)
			break;
        /*
         * We got resized... But check if the window has been already resized (no-reentry)...
         */
        if(window->State.Width != LOWORD(lParam) || window->State.Height != HIWORD(lParam))
		{
			window->State.NeedToResize = GL_TRUE;
			window->State.Width  = LOWORD(lParam);
			window->State.Height = HIWORD(lParam);
			//if(window->Window.Surface != EGL_NO_SURFACE)
				//__glCreateSurface(window);
		}
        break;
#if 0
    case WM_SETFOCUS: 
        printf("WM_SETFOCUS: %p\n", window );
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_ACTIVATE: 
        if (LOWORD(wParam) != WA_INACTIVE)
        {
            /* glutSetCursor( fgStructure.Window->State.Cursor ); */
            printf("WM_ACTIVATE: glutSetCursor( %p, %d)\n", window,
                   window->State.Cursor );
            glutSetCursor( window->State.Cursor );
        }

        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;
#endif

#ifdef _WIN32_WCE
	case WM_HOTKEY:
		{
			int idHotKey = (int) wParam; 
			UINT fuModifiers = (UINT) LOWORD(lParam); 
			UINT uVirtKey = (UINT) HIWORD(lParam);
			static BOOL bPressed = FALSE;
			
			if (!bPressed)
			{
				SendMessage(hWnd, WM_KEYDOWN, wParam, lParam); //This is actually first event
				bPressed = TRUE;
			} 
			else if (fuModifiers & MOD_KEYUP) 
			{
				SendMessage(hWnd, WM_KEYUP, wParam, lParam); //And this is the last
				bPressed = FALSE;
			}
			return 0;

		}
		break;

#endif

	case WM_ACTIVATE:
#ifdef _WIN32_WCE
		// Notify shell of our activate message
		SHHandleWMActivate(hWnd, wParam, lParam, &s_sai, FALSE);

		switch(LOWORD(wParam))	// it isn't inactive
		{
			case WA_ACTIVE:
			case WA_CLICKACTIVE:
				if(((BOOL)HIWORD(wParam)) == FALSE) //it isn't minimized
				{
					if(!window->IsMenu)
					{
						UpdateWindowPosition(window->Window.Handle);
						RegisterHotKeys(hWnd); //Local function
					}
				}
		}
#endif //_WIN32_WCE
		lRet = 0;//DefWindowProc( hWnd, uMsg, wParam, lParam );
		break;
 
#ifdef _WIN32_WCE		
	case WM_SETTINGCHANGE:
		SHHandleWMSettingChange(hWnd, wParam, lParam, &s_sai);
		if(!window->IsMenu)
			UpdateWindowPosition(window->Window.Handle);
		lRet = 0;//DefWindowProc( hWnd, uMsg, wParam, lParam );
		break;
#endif //_WIN32_WCE
        /*
         * XXX Why not re-use some common code with the glutSetCursor()
         * XXX function (or perhaps invoke glutSetCursor())?
         * XXX That is, why are we duplicating code, here, from
         * XXX glutSetCursor()?  The WIN32 code should be able to just
         * XXX call glutSetCurdsor() instead of defining two macros
         * XXX and implementing a nested case in-line.
         */
    case WM_SETCURSOR: 
        /* Set the cursor AND change it for this window class. */
#define MAP_CURSOR(a,b)                 \
    case a:                             \
    SetCursor( LoadCursor( NULL, b ) ); \
    break;

        /* Nuke the cursor AND change it for this window class. */
#define ZAP_CURSOR(a,b) \
    case a:             \
    SetCursor( NULL );  \
    break;

        if( LOWORD( lParam ) == HTCLIENT )
            switch( window->State.Cursor )
            {
                MAP_CURSOR( GLUT_CURSOR_RIGHT_ARROW, IDC_ARROW     );
                MAP_CURSOR( GLUT_CURSOR_LEFT_ARROW,  IDC_ARROW     );
                MAP_CURSOR( GLUT_CURSOR_INFO,        IDC_HELP      );
                MAP_CURSOR( GLUT_CURSOR_DESTROY,     IDC_CROSS     );
                MAP_CURSOR( GLUT_CURSOR_HELP,        IDC_HELP      );
                MAP_CURSOR( GLUT_CURSOR_CYCLE,       IDC_SIZEALL   );
                MAP_CURSOR( GLUT_CURSOR_SPRAY,       IDC_CROSS     );
                MAP_CURSOR( GLUT_CURSOR_WAIT,        IDC_WAIT      );
                MAP_CURSOR( GLUT_CURSOR_TEXT,        IDC_UPARROW   );
                MAP_CURSOR( GLUT_CURSOR_CROSSHAIR,   IDC_CROSS     );
                /* MAP_CURSOR( GLUT_CURSOR_NONE,        IDC_NO         ); */
                ZAP_CURSOR( GLUT_CURSOR_NONE,        NULL          );

            default:
                MAP_CURSOR( GLUT_CURSOR_UP_DOWN,     IDC_ARROW     );
            }
        else
            lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_SHOWWINDOW:
        window->State.Visible = GL_TRUE;
        window->State.Redisplay = GL_TRUE;
        break;

    case WM_PAINT:
        /* Turn on the visibility in case it was turned off somehow */
        window->State.Visible = GL_TRUE;
        BeginPaint( hWnd, &ps );
		if (!fgMappedMenu)
			fghRedrawWindowByHandle( hWnd );
        EndPaint( hWnd, &ps );
		lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_CLOSE:
        fgDestroyWindow ( window );
        if ( fgState.ActionOnWindowClose != GLUT_ACTION_CONTINUE_EXECUTION )
            PostQuitMessage(0);
        break;

    case WM_DESTROY:
        /*
         * The window already got destroyed, so don't bother with it.
         */
        return 0;

    case WM_MOUSEMOVE:
    {
        window->State.MouseX = LOWORD( lParam );
        window->State.MouseY = HIWORD( lParam );
#ifdef USE_GLMENU 
        if ( window->ActiveMenu )
        {
            window->State.Redisplay = GL_TRUE;
            fgSetWindow ( window->ActiveMenu->ParentWindow );
            break;
        }
#endif /* USE_GLMENU */
        fgState.Modifiers = fgGetWin32Modifiers( );

        if( ( wParam & MK_LBUTTON ) ||
            ( wParam & MK_MBUTTON ) ||
            ( wParam & MK_RBUTTON ) )
            INVOKE_WCB( *window, Motion, ( window->State.MouseX,
                                           window->State.MouseY ) );
        else
            INVOKE_WCB( *window, Passive, ( window->State.MouseX,
                                            window->State.MouseY ) );

        fgState.Modifiers = 0xffffffff;
    }
    break;

	case WM_COMMAND:
		fgOnCommand(hWnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam));
		return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
    {
		GLboolean pressed = GL_TRUE;
        int button;

        window->State.MouseX = LOWORD( lParam );
        window->State.MouseY = HIWORD( lParam );

        switch( uMsg )
        {
        case WM_LBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_LEFT_BUTTON;
            break;
        case WM_MBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_MIDDLE_BUTTON;
            break;
        case WM_RBUTTONDOWN:
            pressed = GL_TRUE;
            button = GLUT_RIGHT_BUTTON;
            break;
        case WM_LBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_LEFT_BUTTON;
            break;
        case WM_MBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_MIDDLE_BUTTON;
            break;
        case WM_RBUTTONUP:
            pressed = GL_FALSE;
            button = GLUT_RIGHT_BUTTON;
            break;
        default:
            pressed = GL_FALSE;
            button = -1;
            break;
        }

#ifndef _WIN32_WCE
        if( GetSystemMetrics( SM_SWAPBUTTON ) )
            if( button == GLUT_LEFT_BUTTON )
                button = GLUT_RIGHT_BUTTON;
            else if( button == GLUT_RIGHT_BUTTON )
                button = GLUT_LEFT_BUTTON;
#endif

        if( button == -1 )
            return DefWindowProc( hWnd, uMsg, lParam, wParam );

#ifdef USE_GLMENU
        /*
         * XXX This comment is duplicated in two other spots.
         * XXX Can we centralize it?
         *
         * Do not execute the application's mouse callback if a
         * menu is hooked to this button.
         * In that case an appropriate private call should be generated.
         * Near as I can tell, this is the menu behaviour:
         *  - Down-click the menu button, menu not active:  activate
         *    the menu with its upper left-hand corner at the mouse location.
         *  - Down-click any button outside the menu, menu active:
         *    deactivate the menu
         *  - Down-click any button inside the menu, menu active:
         *    select the menu entry and deactivate the menu
         *  - Up-click the menu button, menu not active:  nothing happens
         *  - Up-click the menu button outside the menu, menu active:
         *    nothing happens
         *  - Up-click the menu button inside the menu, menu active:
         *    select the menu entry and deactivate the menu
         */
        /* Window has an active menu, it absorbs any mouse click */
        if( window->ActiveMenu )
        {
            /* Outside the menu, deactivate the menu if it's a downclick */
            if( ! fgCheckActiveMenu( window, window->ActiveMenu ) )
            {
                if( pressed )
                    fgDeactivateMenu( window->ActiveMenu->ParentWindow );
            }
            else  /* In menu, invoke the callback and deactivate the menu*/
            {
                /*
                 * Save the current window and menu and set the current
                 * window to the window whose menu this is
                 */
                SFG_Window *save_window = fgStructure.Window;
                SFG_Menu *save_menu = fgStructure.Menu;
                SFG_Window *parent_window = window->ActiveMenu->ParentWindow;
                fgSetWindow( parent_window );
                fgStructure.Menu = window->ActiveMenu;

                /* Execute the menu callback */
                fgExecuteMenuCallback( window->ActiveMenu );
                fgDeactivateMenu( parent_window );

                /* Restore the current window and menu */
                fgSetWindow( save_window );
                fgStructure.Menu = save_menu;
            }

            /*
             * Let's make the window redraw as a result of the mouse
             * click and menu activity.
             */
            if( ! window->IsMenu )
                window->State.Redisplay = GL_TRUE;

            break;
        }

        if( window->Menu[ button ] && pressed )
        {
            window->State.Redisplay = GL_TRUE;
            fgSetWindow( window );
            fgActivateMenu( window, button );

            break;
        }
#else // ! USE_GLMENU

		if(pressed)
		{
			if(fgOnButtonDn(hWnd, window->State.MouseX, window->State.MouseY, button))
				break;
		}
		else
		{
			if(fgOnButtonUp(hWnd, window->State.MouseX, window->State.MouseY, button))
				break;
		}
#endif // USE_GLMENU

        if( ! FETCH_WCB( *window, Mouse ) )
            break;

        fgSetWindow( window );
        fgState.Modifiers = fgGetWin32Modifiers( );

        INVOKE_WCB(
            *window, Mouse,
            ( button,
              pressed ? GLUT_DOWN : GLUT_UP,
              window->State.MouseX,
              window->State.MouseY
            )
        );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case 0x020a:
        /* Should be WM_MOUSEWHEEL but my compiler doesn't recognize it */
    {
        /*
         * XXX THIS IS SPECULATIVE -- John Fay, 10/2/03
         * XXX Should use WHEEL_DELTA instead of 120
         */
        int wheel_number = LOWORD( wParam );
        short ticks = ( short )HIWORD( wParam ) / 120;
        int direction = 1;

        if( ticks < 0 )
        {
            direction = -1;
            ticks = -ticks;
        }

        /*
         * The mouse cursor has moved. Remember the new mouse cursor's position
         */
        /*        window->State.MouseX = LOWORD( lParam ); */
        /* Need to adjust by window position, */
        /*        window->State.MouseY = HIWORD( lParam ); */
        /* change "lParam" to other parameter */

        if( ! FETCH_WCB( *window, MouseWheel ) &&
            ! FETCH_WCB( *window, Mouse ) )
            break;

        fgSetWindow( window );
        fgState.Modifiers = fgGetWin32Modifiers( );

        while( ticks-- )
            if( FETCH_WCB( *window, MouseWheel ) )
                INVOKE_WCB( *window, MouseWheel,
                            ( wheel_number,
                              direction,
                              window->State.MouseX,
                              window->State.MouseY
                            )
                );
            else  /* No mouse wheel, call the mouse button callback twice */
            {
                /*
                 * XXX The below assumes that you have no more than 3 mouse
                 * XXX buttons.  Sorry.
                 */
                int button = wheel_number*2 + 4;
                if( direction > 0 )
                    ++button;
                INVOKE_WCB( *window, Mouse,
                            ( button, GLUT_DOWN,
                              window->State.MouseX, window->State.MouseY )
                );
                INVOKE_WCB( *window, Mouse,
                            ( button, GLUT_UP,
                              window->State.MouseX, window->State.MouseX )
                );
            }

        fgState.Modifiers = 0xffffffff;
    }
    break ;

    case WM_SYSKEYDOWN:
    case WM_KEYDOWN:
    {
        int keypress = -1;
        POINT mouse_pos ;

        if( fgState.IgnoreKeyRepeat && (lParam & KF_REPEAT) )
            break;

		// Hack to emulate PocketPC shortcuts keys
#ifdef _WIN32_WCE		
		if((int)wParam == 0xC1) wParam = VK_F1;
		if((int)wParam == 0xC2) wParam = VK_F2;
		if((int)wParam == 0xC3) wParam = VK_F3;
		if((int)wParam == 0xC4) wParam = VK_F4;
#endif //_WIN32_WCE

        /*
         * Remember the current modifiers state. This is done here in order 
         * to make sure the VK_DELETE keyboard callback is executed properly.
         */
        fgState.Modifiers = fgGetWin32Modifiers( );

        GetCursorPos( &mouse_pos );
        ScreenToClient( window->Window.Handle, &mouse_pos );

        window->State.MouseX = mouse_pos.x;
        window->State.MouseY = mouse_pos.y;

        /*
         * Convert the Win32 keystroke codes to GLUTtish way
         */
#       define KEY(a,b) case a: keypress = b; break;

        switch( wParam )
        {
            KEY( VK_F1,     GLUT_KEY_F1        );
            KEY( VK_F2,     GLUT_KEY_F2        );
            KEY( VK_F3,     GLUT_KEY_F3        );
            KEY( VK_F4,     GLUT_KEY_F4        );
            KEY( VK_F5,     GLUT_KEY_F5        );
            KEY( VK_F6,     GLUT_KEY_F6        );
            KEY( VK_F7,     GLUT_KEY_F7        );
            KEY( VK_F8,     GLUT_KEY_F8        );
            KEY( VK_F9,     GLUT_KEY_F9        );
            KEY( VK_F10,    GLUT_KEY_F10       );
            KEY( VK_F11,    GLUT_KEY_F11       );
            KEY( VK_F12,    GLUT_KEY_F12       );
            KEY( VK_PRIOR,  GLUT_KEY_PAGE_UP   );
            KEY( VK_NEXT,   GLUT_KEY_PAGE_DOWN );
            KEY( VK_HOME,   GLUT_KEY_HOME      );
            KEY( VK_END,    GLUT_KEY_END       );
            KEY( VK_LEFT,   GLUT_KEY_LEFT      );
            KEY( VK_UP,     GLUT_KEY_UP        );
            KEY( VK_RIGHT,  GLUT_KEY_RIGHT     );
            KEY( VK_DOWN,   GLUT_KEY_DOWN      );
            KEY( VK_INSERT, GLUT_KEY_INSERT    );

        case VK_DELETE:
            /*
             * The delete key should be treated as an ASCII keypress:
             */
            INVOKE_WCB( *window, Keyboard,
                        ( 127, window->State.MouseX, window->State.MouseY )
            );
			break;
		case VK_RETURN:
            /*
             * The return key should be treated as an ASCII keypress:
             */
            INVOKE_WCB( *window, Keyboard,
                        ( '\n', window->State.MouseX, window->State.MouseY )
            );
			break;
        }

        if( keypress != -1 )
            INVOKE_WCB( *window, Special,
                        ( keypress,
                          window->State.MouseX, window->State.MouseY )
            );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_SYSKEYUP:
    case WM_KEYUP:
    {
        int keypress = -1;
        POINT mouse_pos;

		// Hack to emulate PocketPC shortcuts keys
#ifdef _WIN32_WCE		
		if((int)wParam == 0xC1) wParam = VK_F1;
		if((int)wParam == 0xC2) wParam = VK_F2;
		if((int)wParam == 0xC3) wParam = VK_F3;
		if((int)wParam == 0xC4) wParam = VK_F4;
#endif //_WIN32_WCE
        /*
         * Remember the current modifiers state. This is done here in order 
         * to make sure the VK_DELETE keyboard callback is executed properly.
         */
        fgState.Modifiers = fgGetWin32Modifiers( );

        GetCursorPos( &mouse_pos );
        ScreenToClient( window->Window.Handle, &mouse_pos );

        window->State.MouseX = mouse_pos.x;
        window->State.MouseY = mouse_pos.y;

        /*
         * Convert the Win32 keystroke codes to GLUTtish way.
         * "KEY(a,b)" was defined under "WM_KEYDOWN"
         */

        switch( wParam )
        {
            KEY( VK_F1,     GLUT_KEY_F1        );
            KEY( VK_F2,     GLUT_KEY_F2        );
            KEY( VK_F3,     GLUT_KEY_F3        );
            KEY( VK_F4,     GLUT_KEY_F4        );
            KEY( VK_F5,     GLUT_KEY_F5        );
            KEY( VK_F6,     GLUT_KEY_F6        );
            KEY( VK_F7,     GLUT_KEY_F7        );
            KEY( VK_F8,     GLUT_KEY_F8        );
            KEY( VK_F9,     GLUT_KEY_F9        );
            KEY( VK_F10,    GLUT_KEY_F10       );
            KEY( VK_F11,    GLUT_KEY_F11       );
            KEY( VK_F12,    GLUT_KEY_F12       );
            KEY( VK_PRIOR,  GLUT_KEY_PAGE_UP   );
            KEY( VK_NEXT,   GLUT_KEY_PAGE_DOWN );
            KEY( VK_HOME,   GLUT_KEY_HOME      );
            KEY( VK_END,    GLUT_KEY_END       );
            KEY( VK_LEFT,   GLUT_KEY_LEFT      );
            KEY( VK_UP,     GLUT_KEY_UP        );
            KEY( VK_RIGHT,  GLUT_KEY_RIGHT     );
            KEY( VK_DOWN,   GLUT_KEY_DOWN      );
            KEY( VK_INSERT, GLUT_KEY_INSERT    );

          case VK_DELETE:
              /*
               * The delete key should be treated as an ASCII keypress:
               */
              INVOKE_WCB( *window, KeyboardUp,
                          ( 127, window->State.MouseX, window->State.MouseY )
              );
              break;
		  case VK_RETURN:
			  /*
               * The return key should be treated as an ASCII keypress:
               */
              INVOKE_WCB( *window, KeyboardUp,
                          ( '\n', window->State.MouseX, window->State.MouseY )
              );

        default:
        {
			WORD code[ 2 ];
#ifndef _WIN32_WCE
            BYTE state[ 256 ];
            
            GetKeyboardState( state );
            
            if( ToAscii( wParam, 0, state, code, 0 ) == 1 )
#else
			code[ 0 ] = wParam;
			if(wParam < '0' || wParam > 'Z')
				wParam = 0;
			else
#endif
                wParam = code[ 0 ];


            INVOKE_WCB( *window, KeyboardUp,
                        ( (char)wParam,
                          window->State.MouseX, window->State.MouseY )
            );
        }
        }

        if( keypress != -1 )
            INVOKE_WCB( *window, SpecialUp,
                        ( keypress,
                          window->State.MouseX, window->State.MouseY )
            );

        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_SYSCHAR:
    case WM_CHAR:
    {
        if( fgState.IgnoreKeyRepeat && (lParam & KF_REPEAT) )
            break;

        fgState.Modifiers = fgGetWin32Modifiers( );
        INVOKE_WCB( *window, Keyboard,
                    ( (char)wParam,
                      window->State.MouseX, window->State.MouseY )
        );
        fgState.Modifiers = 0xffffffff;
    }
    break;

    case WM_CAPTURECHANGED:
        /* User has finished resizing the window, force a redraw */
		if (!fgMappedMenu)
			INVOKE_WCB( *window, Display, ( ) );

        /*lRet = DefWindowProc( hWnd, uMsg, wParam, lParam ); */
        break;

        /*
         * Other messages that I have seen and which are not handled already
         */
    case WM_SETTEXT:  /* 0x000c */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Pass it on to "DefWindowProc" to set the window text */
        break;

    case WM_GETTEXT:  /* 0x000d */
        /* Ideally we would copy the title of the window into "lParam" */
        /* strncpy ( (char *)lParam, "Window Title", wParam );
           lRet = ( wParam > 12 ) ? 12 : wParam;  */
        /* the number of characters copied */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

    case WM_GETTEXTLENGTH:  /* 0x000e */
        /* Ideally we would get the length of the title of the window */
        lRet = 12;
        /* the number of characters in "Window Title\0" (see above) */
        break;

    case WM_ERASEBKGND:  /* 0x0014 */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;

#ifndef _WIN32_WCE
    case WM_SYNCPAINT:  /* 0x0088 */
        /* Another window has moved, need to update this one */
        window->State.Redisplay = GL_TRUE;
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Help screen says this message must be passed to "DefWindowProc" */
        break;

    case WM_NCPAINT:  /* 0x0085 */
      /* Need to update the border of this window */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        /* Pass it on to "DefWindowProc" to repaint a standard border */
        break;

    case WM_SYSCOMMAND :  /* 0x0112 */
        {
          /*
           * We have received a system command message.  Try to act on it.
           * The commands are passed in through the "lParam" parameter:
           * Clicking on a corner to resize the window gives a "F004" message
           * but this is not defined in my header file.
           */
            switch ( lParam )
            {
            case SC_SIZE       :
                break ;

            case SC_MOVE       :
                break ;

            case SC_MINIMIZE   :
                /* User has clicked on the "-" to minimize the window */
                /* Turn off the visibility */
                window->State.Visible = GL_FALSE ;

                break ;

            case SC_MAXIMIZE   :
                break ;

            case SC_NEXTWINDOW :
                break ;

            case SC_PREVWINDOW :
                break ;

            case SC_CLOSE      :
                /* Followed very closely by a WM_CLOSE message */
                break ;

            case SC_VSCROLL    :
                break ;

            case SC_HSCROLL    :
                break ;

            case SC_MOUSEMENU  :
                break ;

            case SC_KEYMENU    :
                break ;

            case SC_ARRANGE    :
                break ;

            case SC_RESTORE    :
                break ;

            case SC_TASKLIST   :
                break ;

            case SC_SCREENSAVE :
                break ;

            case SC_HOTKEY     :
                break ;
            }
        }

        /* We need to pass the message on to the operating system as well */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;
#endif
    default:
        /*
         * Handle unhandled messages
         */
        lRet = DefWindowProc( hWnd, uMsg, wParam, lParam );
        break;
    }

    return lRet;
}
#endif


#ifdef GLUTES_STATIC

extern int main(int argc, const char *argv[]);

#define MAX_ARGS 64

#ifdef _WIN32_WCE

int WINAPI WinMain(	HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPWSTR    lpCmdLine,
				   int       nCmdShow)
{
	const char * argv[MAX_ARGS];
#ifdef UNICODE
	WCHAR szExeName[MAX_PATH], szTempName[MAX_PATH];
#else
	char szExeName[MAX_PATH], szTempName[MAX_PATH];
#endif
	int argc = 1, size;
	char *str = 0, *cname = 0;
	int i, len;
	HWND hWnd = FindWindow(TEXT("GLUT|ES"), NULL);	
	if (hWnd) 
	{
		// set focus to foremost child window
		// The "| 0x01" is used to bring any owned windows to the foreground and
		// activate them.
		SetForegroundWindow((HWND)((ULONG) hWnd | 0x00000001));
		return 0;
	}
	
	len = GetModuleFileName(NULL, szExeName, MAX_PATH);
	_tcscpy(szTempName, szExeName);
	for(i = 0; i < len; i++) 
		if(szTempName[i]=='\\') 
			szTempName[i]='/';
#ifdef UNICODE
	/* convert unicode string to ansi */
	size = WideCharToMultiByte(CP_ACP, 0, szExeName, -1, cname, 0, NULL, NULL);
	cname = (char*)malloc(size);
	WideCharToMultiByte(CP_ACP, 0, szExeName, -1, cname, size, NULL, NULL);
#else
	cname= szExeName;
#endif

	argv[0] = cname;
		
#ifdef UNICODE
	/* convert unicode string to ansi */
	size = WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, str, 0, NULL, NULL);
	str = (char*)malloc(size);
	WideCharToMultiByte(CP_ACP, 0, lpCmdLine, -1, str, size, NULL, NULL);
#else
	str = lpCmdLine;
#endif
	/* break ansi string into components */
		
	/*pch = strtok(str," ");
		
	while (pch != NULL) 
	{
		argv[argc++] = pch;
		pch = strtok(NULL, " ");
	}
		
	if(argc>1)
	{
		fname += argv[1];
		for(int i = 2; i < argc; i++) 
		{
			fname += " ";
			fname += argv[i];				
		}
	}*/

	/* in oder to not break filepath, only one parameter is given */
	if(strcmp(str, ""))
	{
		argc = 2;
		argv[1] = str;
	}

	return main(argc, (const char**)argv);
}

#else

int WINAPI WinMain(HINSTANCE hInstance,
				   HINSTANCE hPrevInstance,
				   LPSTR    lpCmdLine,
				   int       nCmdShow)
{
	return main(__argc,(const char**) __argv);
}

#endif

#endif //GLUTES_STATIC
/*** END OF FILE ***/
