/*
 * glutes_internal.h
 *
 * The GLUT|ES library private include file.
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef  glutes_internal_H
#define  glutes_internal_H

/* XXX Update these for each release! */
#define  VERSION_MAJOR 2
#define  VERSION_MINOR 2
#define  VERSION_PATCH 0 

/*
 * Freeglut is meant to be available under all Unix/X11 and Win32 platforms.
 */
#if !defined(_WIN32)
#   define  TARGET_HOST_UNIX_X11    1
#   define  TARGET_HOST_WIN32       0
#   define  USE_GLMENU
#   define  USE_EGL
#else
#   define  TARGET_HOST_UNIX_X11    0
#   define  TARGET_HOST_WIN32       1
#endif

#define  FREEGLUT_MAX_MENUS         3
#define  FREEGLUT_DEBUG             1

#if FREEGLUT_DEBUG
    #undef   G_DISABLE_ASSERT
    #undef   G_DISABLE_CHECKS
#else
    #define  G_DISABLE_ASSERT
    #define  G_DISABLE_CHECKS
#endif

#define _INT2FIXED(_x_) (GLfixed)((_x_) << 16)
#define _FIXED2INT(_x_) (int)((_x_) >> 16)
#define _FLOAT2FIXED(_x_) (GLfixed)((_x_) *  (float)(1 << 16) + 0.5f)
#define _FIXED2FLOAT(_x_) ((float)(_x_) / (float)(1 << 16))

/*
 * Somehow all Win32 include headers depend on this one:
 */
#if TARGET_HOST_WIN32
#include <windows.h>
#include <windowsx.h>

#define strdup   _strdup
#endif

/*
 * Those files should be available on every platform.
 */
#include <GLES/gl.h>
#include <GLES/egl.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#if TARGET_HOST_UNIX_X11
#include <unistd.h>
#include <sys/time.h>
#endif

/*
 * The system-dependant include files should go here:
 */
#if TARGET_HOST_UNIX_X11
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
    #include <X11/Xutil.h>
    #include <X11/keysym.h>

    #ifdef HAVE_X11_EXTENSIONS_XF86VMODE_H
    #include <X11/extensions/xf86vmode.h>
    #endif
#endif

/*
 * Microsoft VisualC++ 5.0's <math.h> does not define the PI
 */
#ifndef M_PI
#    define  M_PI  3.14159265358979323846
#endif

#ifndef TRUE
#    define  TRUE  1
#endif

#ifndef FALSE
#    define  FALSE  0
#endif

/* -- GLOBAL TYPE DEFINITIONS ---------------------------------------------- */

/*
 * Freeglut callbacks type definitions
 */
typedef void (* FGCBDisplay       )( void );
typedef void (* FGCBReshape       )( int, int );
typedef void (* FGCBVisibility    )( int );
typedef void (* FGCBKeyboard      )( unsigned char, int, int );
typedef void (* FGCBSpecial       )( int, int, int );
typedef void (* FGCBMouse         )( int, int, int, int );
typedef void (* FGCBMouseWheel    )( int, int, int, int );
typedef void (* FGCBMotion        )( int, int );
typedef void (* FGCBPassive       )( int, int );
typedef void (* FGCBEntry         )( int );
typedef void (* FGCBWindowStatus  )( int );
typedef void (* FGCBSelect        )( int, int, int );
typedef void (* FGCBJoystick      )( unsigned int, int, int, int );
typedef void (* FGCBKeyboardUp    )( unsigned char, int, int );
typedef void (* FGCBSpecialUp     )( int, int, int );
typedef void (* FGCBOverlayDisplay)( void );
typedef void (* FGCBSpaceMotion   )( int, int, int );
typedef void (* FGCBSpaceRotation )( int, int, int );
typedef void (* FGCBSpaceButton   )( int, int );
typedef void (* FGCBDials         )( int, int );
typedef void (* FGCBButtonBox     )( int, int );
typedef void (* FGCBTabletMotion  )( int, int );
typedef void (* FGCBTabletButton  )( int, int, int, int );
typedef void (* FGCBDestroy       )( void );

/*
 * The global callbacks type definitions
 */
typedef void (* FGCBIdle          )( void );
typedef void (* FGCBTimer         )( int );
typedef void (* FGCBMenuState     )( int );
typedef void (* FGCBMenuStatus    )( int, int, int );

/*
 * The callback used when creating/using menus
 */
typedef void (* FGCBMenu          )( int );


/*
 * A list structure
 */
typedef struct tagSFG_List SFG_List;
struct tagSFG_List
{
    void *First;
    void *Last;
};

/*
 * A list node structure
 */
typedef struct tagSFG_Node SFG_Node;
struct tagSFG_Node
{
    void *Next;
    void *Prev;
};

/*
 * A helper structure holding two ints and a boolean
 */
typedef struct tagSFG_XYUse SFG_XYUse;
struct tagSFG_XYUse
{
    GLint           X, Y;               /* The two integers...               */
    GLboolean       Use;                /* ...and a single boolean.          */
};

/*
 * A helper structure holding a timeval and a boolean
 */
typedef struct tagSFG_Time SFG_Time;
struct tagSFG_Time
{
#ifdef WIN32
    DWORD Value;
#else
    struct timeval  Value;
#endif
    GLboolean       Set;
};

/*
 * An enumeration containing the state of the GLUT execution:
 * initializing, running, or stopping
 */
typedef enum
{
  GLUT_EXEC_STATE_INIT,
  GLUT_EXEC_STATE_RUNNING,
  GLUT_EXEC_STATE_STOP
} fgExecutionState ;

/*
 * This structure holds different freeglut settings
 */
typedef struct tagSFG_State SFG_State;
struct tagSFG_State
{
    SFG_XYUse        Position;             /* The default windows' position  */
    SFG_XYUse        Size;                 /* The default windows' size      */
    unsigned int     DisplayMode;          /* Display mode for new windows   */

    GLboolean        Initialised;          /* freeglut has been initialised  */

    GLboolean        ForceDirectContext;   /* Force direct rendering?        */
    GLboolean        TryDirectContext;     /* What about giving a try to?    */

    GLboolean        ForceIconic;          /* New top windows are iconified  */
    GLboolean        UseCurrentContext;    /* New windows share with current */

    GLboolean        GLDebugSwitch;        /* OpenGL state debugging switch  */
    GLboolean        XSyncSwitch;          /* X11 sync protocol switch       */

    GLboolean        IgnoreKeyRepeat;      /* Whether to ignore key repeat.  */
    int              Modifiers;            /* Current ALT/SHIFT/CTRL state   */

    GLuint           FPSInterval;          /* Interval between FPS printfs   */
    GLuint           SwapCount;            /* Count of glutSwapBuffer calls  */
    GLuint           SwapTime;             /* Time of last SwapBuffers       */

    SFG_Time         Time;                 /* Time that glutInit was called  */
    SFG_List         Timers;               /* The freeglut timer hooks       */
    SFG_List         FreeTimers;           /* The unused timer hooks         */

    FGCBIdle         IdleCallback;         /* The global idle callback       */

    int              ActiveMenus;          /* Num. of currently active menus */
    FGCBMenuState    MenuStateCallback;    /* Menu callbacks are global      */
    FGCBMenuStatus   MenuStatusCallback;

    SFG_XYUse        GameModeSize;         /* Game mode screen's dimensions  */
    int              GameModeDepth;        /* The pixel depth for game mode  */
    int              GameModeRefresh;      /* The refresh rate for game mode */

    int              ActionOnWindowClose; /* Action when user closes window  */

    fgExecutionState ExecState;           /* Used for GLUT termination       */
    char            *ProgramName;         /* Name of the invoking program    */
};

#ifdef _WIN32_WCE

#define FBI_FORMAT_565		1
#define FBI_FORMAT_555		2
#define FBI_FORMAT_OTHER	3

typedef struct _RawFrameBufferInfo
{
	WORD wFormat;	
	WORD wBPP;
	VOID *pFramePointer;
	int cxStride;
	int cyStride;
	int cxPixels;
	int cyPixels;
} RawFrameBufferInfo;

#endif


/*
 * The structure used by display initialization in glutes_init.c
 */
typedef struct tagSFG_Display SFG_Display;
struct tagSFG_Display
{
	EGLDisplay		eglDisplay;
	int				PixmapColorFormat;
#if TARGET_HOST_UNIX_X11
    Display*        Display;            /* The display we are being run in.  */
    int             Screen;             /* The screen we are about to use.   */
    Window          RootWindow;         /* The screen's root window.         */
    int             Connection;         /* The display's connection number   */
    Atom            DeleteWindow;       /* The window deletion atom          */

#ifdef X_XF86VidModeGetModeLine
    /*
     * XF86VidMode may be compilable even if it fails at runtime.  Therefore,
     * the validity of the VidMode has to be tracked
     */
    int             DisplayModeValid;   /* Flag that indicates runtime status*/
    XF86VidModeModeLine DisplayMode;    /* Current screen's display settings */
    int             DisplayModeClock;   /* The display mode's refresh rate   */
    int             DisplayViewPortX;   /* saved X location of the viewport  */
    int             DisplayViewPortY;   /* saved Y location of the viewport  */
    int             DisplayPointerX;    /* saved X location of the pointer   */
    int             DisplayPointerY;    /* saved Y location of the pointer   */

#endif

#elif TARGET_HOST_WIN32
    HINSTANCE       Instance;           /* The application's instance        */
    DEVMODE         DisplayMode;        /* Desktop's display settings        */
#ifdef _WIN32_WCE
	RawFrameBufferInfo FrameBufferInfo;
#endif

#endif


    int             ScreenWidth;        /* The screen's width in pixels      */
    int             ScreenHeight;       /* The screen's height in pixels     */
    int             ScreenWidthMM;      /* The screen's width in milimeters  */
    int             ScreenHeightMM;     /* The screen's height in milimeters */
};


/*
 * The user can create any number of timer hooks
 */
typedef struct tagSFG_Timer SFG_Timer;
struct tagSFG_Timer
{
    SFG_Node        Node;
    int             ID;                 /* The timer ID integer              */
    FGCBTimer       Callback;           /* The timer callback                */
    long            TriggerTime;        /* The timer trigger time            */
};

/*
 * Make "freeglut" window handle and context types so that we don't need so
 * much conditionally-compiled code later in the library.
 */
#if TARGET_HOST_UNIX_X11

typedef Window		SFG_WindowHandleType ;
typedef EGLContext	SFG_WindowContextType ;
typedef long		DWORD ;

#elif TARGET_HOST_WIN32

typedef HWND		SFG_WindowHandleType ;
typedef EGLContext	SFG_WindowContextType ;

#endif

/*
 * A window and its OpenGL context. The contents of this structure
 * are highly dependant on the target operating system we aim at...
 */
typedef struct tagSFG_Context SFG_Context;
struct tagSFG_Context
{
    SFG_WindowHandleType  Handle;    /* The window's handle                 */
    SFG_WindowContextType Context;				/* The window's OpenGL|ES EGL context     */

	EGLSurface Surface;
	EGLConfig eglConfig;
	int SurfaceType;
	DWORD *bits;

#if TARGET_HOST_UNIX_X11
    XVisualInfo*    VisualInfo;      /* The window's visual information     */
#elif TARGET_HOST_WIN32
    HBITMAP  pixmap;
    HDC      Device;          /* The window's device context         */
#endif

    int             DoubleBuffered;  /* Treat the window as double-buffered */
};

/*
 * Window's state description. This structure should be kept portable.
 */
typedef struct tagSFG_WindowState SFG_WindowState;
struct tagSFG_WindowState
{
    int             Width;              /* Window's width in pixels          */
    int             Height;             /* The same about the height         */
    int             OldWidth;           /* Window width from before a resize */
    int             OldHeight;          /*   "    height  "    "    "   "    */

    GLboolean       Redisplay;          /* Do we have to redisplay?          */
    GLboolean       Visible;            /* Is the window visible now         */

    int             Cursor;             /* The currently selected cursor     */

    long            JoystickPollRate;   /* The joystick polling rate         */
    long            JoystickLastPoll;   /* When the last poll has happened   */

    int             MouseX, MouseY;     /* The most recent mouse position    */

    GLboolean       IsGameMode;         /* Is this the game mode window?     */

    GLboolean       NeedToResize;       /* Do we need to resize the window?  */
};


/*
 * FETCH_WCB() is used as:
 *
 *     FETCH_WCB( window, Visibility );
 *
 * ...where {window} is the freeglut window to fetch the callback from,
 *          {Visibility} is the window-specific callback to fetch.
 *
 * The result is correctly type-cast to the callback function pointer
 * type.
 */
#define FETCH_WCB(window,cbname) \
(*((FGCB ## cbname *)&((window).CallBacks[CB_ ## cbname])))

/*
 * INVOKE_WCB() is used as:
 *
 *     INVOKE_WCB( window, Visibility, ( status ) );
 *
 * ...where {window} is the freeglut window,
 *          {Visibility} is the window-specific callback,
 *          {(status)} is the parameter list.
 *
 * The callback is invoked as:
 *
 *    callback( status );
 *
 * ...so the parentheses are REQUIRED in the {arg_list}.
 *
 * NOTE that it does a sanity-check and also sets the
 * current window.
 *
 */
#define INVOKE_WCB(window,cbname,arg_list)    \
do                                            \
{                                             \
    if( FETCH_WCB( window, cbname ) )         \
    {                                         \
        fgSetWindow( &window );               \
        FETCH_WCB( window, cbname ) arg_list; \
    }                                         \
} while( 0 )

/*
 * The window callbacks the user can supply us with. Should be kept portable.
 *
 * This enumeration provides the freeglut CallBack numbers.
 * The symbolic constants are indices into a window's array of
 * function callbacks.  The names are formed by splicing a common
 * prefix onto the callback's base name.  (This was originally
 * done so that an early stage of development could live side-by-
 * side with the old callback code.  The old callback code used
 * the bare callback's name as a structure member, so I used a
 * prefix for the array index name.)
 *
 * XXX For consistancy, perhaps the prefix should match the
 * XXX FETCH* and INVOKE* macro suffices.  I.e., WCB_, rather than
 * XXX CB_.
 */
enum
{
    CB_Display,
    CB_Reshape,
    CB_Keyboard,
    CB_KeyboardUp,
    CB_Special,
    CB_SpecialUp,
    CB_Mouse,
    CB_MouseWheel,
    CB_Motion,
    CB_Passive,
    CB_Entry,
    CB_Visibility,
    CB_WindowStatus,
    CB_Joystick,
    CB_Destroy,

    /* Presently ignored */
    CB_Select,
    CB_OverlayDisplay,
    CB_SpaceMotion,
    CB_SpaceRotation,
    CB_SpaceButton,
    CB_Dials,
    CB_ButtonBox,
    CB_TabletMotion,
    CB_TabletButton,
    
    /* Always make this the LAST one */
    TOTAL_CALLBACKS
};


/*
 * This structure holds the OpenGL rendering context for all the menu windows
 */
typedef struct tagSFG_MenuContext SFG_MenuContext;
struct tagSFG_MenuContext
{
#if TARGET_HOST_UNIX_X11
    XVisualInfo*        VisualInfo;       /* The window's visual information */
#endif

    SFG_WindowContextType Context;        /* The menu window's WGL context   */
};

/*
 * This structure describes a menu
 */
typedef struct tagSFG_Window SFG_Window;
typedef struct tagSFG_MenuEntry SFG_MenuEntry;
typedef struct tagSFG_Menu SFG_Menu;
struct tagSFG_Menu
{
    SFG_Node            Node;
    void               *UserData;     /* User data passed back at callback   */
    int                 ID;           /* The global menu ID                  */
    SFG_List            Entries;      /* The menu entries list               */
    FGCBMenu            Callback;     /* The menu callback                   */
    FGCBDestroy         Destroy;      /* Destruction callback                */
    GLboolean           IsActive;     /* Is the menu selected?               */
    int                 Width;        /* Menu box width in pixels            */
    int                 Height;       /* Menu box height in pixels           */
    int                 X, Y;         /* Menu box raster position            */

    SFG_MenuEntry      *ActiveEntry;  /* Currently active entry in the menu  */
    SFG_Window         *Window;       /* Window for menu                     */
    SFG_Window         *ParentWindow; /* Window in which the menu is defined */
};

/*
 * This is a menu entry
 */
struct tagSFG_MenuEntry
{
    SFG_Node            Node;
    int                 ID;                     /* The menu entry ID (local) */
    int                 Ordinal;                /* The menu's ordinal number */
    char*               Text;                   /* The text to be displayed  */
    SFG_Menu*           SubMenu;                /* Optional sub-menu tree    */
    GLboolean           IsActive;               /* Is the entry highlighted? */
    int                 Width;                  /* Label's width in pixels   */
};

#ifndef USE_GLMENU

typedef struct tagSFG_WinMenu SFG_WinMenu;
typedef struct tagSFG_WinMenuItem SFG_WinMenuItem;

struct tagSFG_WinMenu 
{
  int					ID;						/* small integer menu id (0-based) */
#if TARGET_HOST_WIN32
  HMENU					Handle;					/* Win32 menu */
#endif
  FGCBMenu				Select;					/*  function of menu */
  SFG_WinMenuItem		*List;					/* list of menu entries */
  int					Num;					/* number of entries */
  void					*UserData ; 
  int					SubMenus;				/* number of submenu entries */
  SFG_WinMenuItem		*Highlighted;			/* pointer to highlighted menu
													entry, NULL not highlighted */
  SFG_WinMenu			*Cascade;				/* currently cascading this menu  */
  SFG_WinMenuItem		*Anchor;				/* currently anchored to this entry */
  int					x;						/* current x origin relative to the
													root window */
  int					y;						/* current y origin relative to the
													root window */
};

struct tagSFG_WinMenuItem 
{
#if TARGET_HOST_WIN32
  HMENU					 Handle;				/* Win32 window for entry */
  UINT					Unique;					/* unique menu item id (Win32 only) */
#endif
  SFG_WinMenu			*Menu;					/* menu entry belongs to */
  int					IsTrigger;				/* is a submenu trigger? */
  int					Value;					/* value to return for selecting this
													entry; doubles as submenu id
													(0-base) if submenu trigger */
  char					*Label;					/* __glutStrdup'ed label string */
  int					Len;					/* length of label string */
  int					PixWidth;				/* width of X window in pixels */
  SFG_WinMenuItem		*Next;					/* next menu entry on list for menu */
};

#endif // USE_GLMENU

/*
 * A window, making part of freeglut windows hierarchy.
 * Should be kept portable.
 */
struct tagSFG_Window
{
    SFG_Node            Node;
    int                 ID;                     /* Window's ID number        */

    SFG_Context         Window;                 /* Window and OpenGL context */
    SFG_WindowState     State;                  /* The window state          */
    void				*CallBacks[ TOTAL_CALLBACKS ]; /* Array of window callbacks */
    void				*UserData ;              /* For use by user           */

#ifdef USE_GLMENU
    SFG_Menu*			Menu[ FREEGLUT_MAX_MENUS ]; /* Menus appended to window  */
    SFG_Menu*			ActiveMenu;                 /* The window's active menu  */
#else
#	define				FG_MAX_MENUS 3
	int					ButtonUses;					/* Number of button uses     */	
	int					Menu[ FG_MAX_MENUS ];		/* Attached menu nums        */
#endif
    SFG_Window*         Parent;                 /* The parent to this window */
    SFG_List            Children;               /* The subwindows d.l. list  */

    GLboolean           IsMenu;                 /* Set to 1 if we are a menu */

};


/*
 * A linked list structure of windows
 */
typedef struct tagSFG_WindowList SFG_WindowList ;
struct tagSFG_WindowList
{
    SFG_Window *window ;
    SFG_WindowList *next ;
};

/*
 * This holds information about all the windows, menus etc.
 */
typedef struct tagSFG_Structure SFG_Structure;
struct tagSFG_Structure
{
    SFG_List        Windows;      /* The global windows list            */
    SFG_List        Menus;        /* The global menus list              */

    SFG_Window*     Window;       /* The currently active win.          */
    SFG_Menu*       Menu;         /* Same, but menu...                  */

    SFG_MenuContext* MenuContext; /* OpenGL rendering context for menus */

    SFG_Window*      GameMode;    /* The game mode window               */

    int              WindowID;    /* The new current window ID          */
    int              MenuID;      /* The new current menu ID            */
};

/*
 * This structure is used for the enumeration purposes.
 * You can easily extend its functionalities by declaring
 * a structure containing enumerator's contents and custom
 * data, then casting its pointer to (SFG_Enumerator *).
 */
typedef struct tagSFG_Enumerator SFG_Enumerator;
struct tagSFG_Enumerator
{
    GLboolean   found;                          /* Used to terminate search  */
    void*       data;                           /* Custom data pointer       */
};
typedef void (* FGCBenumerator  )( SFG_Window *, SFG_Enumerator * );

/*
 * The bitmap font structure
 */
typedef struct tagSFG_Font SFG_Font;
struct tagSFG_Font
{
    char*           Name;         /* The source font name             */
    int             Quantity;     /* Number of chars in font          */
    int             Height;       /* Height of the characters         */
    const GLubyte** Characters;   /* The characters mapping           */

    float           xorig, yorig; /* Relative origin of the character */
};

/*
 * The stroke font structures
 */

typedef struct tagSFG_StrokeVertex SFG_StrokeVertex;
struct tagSFG_StrokeVertex
{
	GLfixed			X, Y;
    //GLfloat         X, Y;
};

typedef struct tagSFG_StrokeStrip SFG_StrokeStrip;
struct tagSFG_StrokeStrip
{
    int             Number;
    const SFG_StrokeVertex* Vertices;
};

typedef struct tagSFG_StrokeChar SFG_StrokeChar;
struct tagSFG_StrokeChar
{
	GLfixed			Right;
    //GLfloat         Right;
    int             Number;
    const SFG_StrokeStrip* Strips;
};

typedef struct tagSFG_StrokeFont SFG_StrokeFont;
struct tagSFG_StrokeFont
{
    char*           Name;                       /* The source font name      */
    int             Quantity;                   /* Number of chars in font   */
	GLfixed			Height;
    //GLfloat         Height;                     /* Height of the characters  */
    const SFG_StrokeChar** Characters;          /* The characters mapping    */
};

/* -- GLOBAL VARIABLES EXPORTS --------------------------------------------- */

/*
 * Freeglut display related stuff (initialized once per session)
 */
extern SFG_Display fgDisplay;

/*
 * Freeglut internal structure
 */
extern SFG_Structure fgStructure;

/*
 * The current freeglut settings
 */
extern SFG_State fgState;

/*
 * The current EGL config 
 */
extern EGLint fgEGLConfigAttribs[];


/* -- PRIVATE FUNCTION DECLARATIONS ---------------------------------------- */

/*
 * A call to this function makes us sure that the Display and Structure
 * subsystems have been properly initialized and are ready to be used
 */
#define  glutes_assert_ready  assert( fgState.Initialised );

/*
 * Following definitions are somewhat similiar to GLib's,
 * but do not generate any log messages:
 */
#define  glutes_return_if_fail( expr ) \
    if( !(expr) )                        \
        return;
#define  glutes_return_val_if_fail( expr, val ) \
    if( !(expr) )                                 \
        return val ;

/*
 * A call to those macros assures us that there is a current
 * window and menu set, respectively:
 */
#define  glutes_assert_window assert( fgStructure.Window != NULL );
#define  glutes_assert_menu   assert( fgStructure.Menu != NULL );

/*
 * The initialize and deinitialize functions get called on glutInit()
 * and glutMainLoop() end respectively. They should create/clean up
 * everything inside of the freeglut
 */
void fgInitialize( const char* displayName );
void fgDeinitialize( void );

/*
 * Those two functions are used to create/destroy the freeglut internal
 * structures. This actually happens when calling glutInit() and when
 * quitting the glutMainLoop() (which actually happens, when all windows
 * have been closed).
 */
void fgCreateStructure( void );
void fgDestroyStructure( void );

/*
 * A helper function to check if a display mode is possible to use
 */
#if TARGET_HOST_UNIX_X11
XVisualInfo* fgChooseVisual( void );
#endif

/*
 * The window procedure for Win32 events handling
 */
#if TARGET_HOST_WIN32
LRESULT CALLBACK fgWindowProc( HWND hWnd, UINT uMsg,
                               WPARAM wParam, LPARAM lParam );
GLboolean fgSetupPixelFormat( SFG_Window* window, GLboolean checkOnly,
                              unsigned char layer_type );
#endif

/*
 * Window creation, opening, closing and destruction.
 * Also CallBack clearing/initialization.
 * Defined in glutes_structure.c, glutes_window.c.
 */
SFG_Window* fgCreateWindow( SFG_Window* parent, const char* title,
                            int x, int y, int w, int h,
                            GLboolean gameMode, GLboolean isMenu );
void        fgSetWindow ( SFG_Window *window );
void        fgOpenWindow( SFG_Window* window, const char* title,
                          int x, int y, int w, int h, GLboolean gameMode,
                          GLboolean isSubWindow );
void        fgCloseWindow( SFG_Window* window );
void        fgAddToWindowDestroyList ( SFG_Window* window );
void        fgCloseWindows ();
void        fgDestroyWindow( SFG_Window* window );
void        fgClearCallBacks( SFG_Window *window );

/*
 * Joystick device management functions, defined in glutes_joystick.c
 */
void        fgJoystickInit( int ident );
void        fgJoystickClose( void );
void        fgJoystickPollWindow( SFG_Window* window );

/*
 * Helper function to enumerate through all registered windows
 * and one to enumerate all of a window's subwindows...
 *
 * The GFunc callback for those functions will be defined as:
 *
 *      void enumCallback( gpointer window, gpointer enumerator );
 *
 * where window is the enumerated (sub)window pointer (SFG_Window *),
 * and userData is the a custom user-supplied pointer. Functions
 * are defined and exported from glutes_structure.c file.
 */
void fgEnumWindows( FGCBenumerator enumCallback, SFG_Enumerator* enumerator );
void fgEnumSubWindows( SFG_Window* window, FGCBenumerator enumCallback,
                       SFG_Enumerator* enumerator );

/*
 * fgWindowByHandle returns a (SFG_Window *) value pointing to the
 * first window in the queue matching the specified window handle.
 * The function is defined in glutes_structure.c file.
 */
SFG_Window* fgWindowByHandle( SFG_WindowHandleType hWindow );

/*
 * This function is similiar to the previous one, except it is
 * looking for a specified (sub)window identifier. The function
 * is defined in glutes_structure.c file.
 */
SFG_Window* fgWindowByID( int windowID );

#ifdef USE_GLMENU

/*
 * Menu creation and destruction. Defined in glutes_structure.c
 */
SFG_Menu*   fgCreateMenu( FGCBMenu menuCallback );
void        fgDestroyMenu( SFG_Menu* menu );

/*
 * Looks up a menu given its ID. This is easier than fgWindowByXXX
 * as all menus are placed in a single doubly linked list...
 */
SFG_Menu* fgMenuByID( int menuID );

/*
 * The menu activation and deactivation the code. This is the meat
 * of the menu user interface handling code...
 */
void fgActivateMenu( SFG_Window* window, int button );
void fgExecuteMenuCallback( SFG_Menu* menu );
GLboolean fgCheckActiveMenu ( SFG_Window *window, SFG_Menu *menu );
void fgDeactivateMenu( SFG_Window *window );
void fgDeactivateSubMenu( SFG_MenuEntry *menuEntry );

/*
 * This function gets called just before the buffers swap, so that
 * freeglut can display the pull-down menus via OpenGL. The function
 * is defined in glutes_menu.c file.
 */
void fgDisplayMenu( void );

#else // ! USE_GLMENU

#if TARGET_HOST_WIN32
extern SFG_WinMenu *fgGetMenu(HMENU win);
extern SFG_WinMenu *fgGetMenuByNum(int menunum);
extern SFG_WinMenuItem *fgGetMenuItem(SFG_WinMenu * menu, HMENU win, int *which);
extern SFG_WinMenuItem * fgGetUniqueMenuItem(SFG_WinMenu * menu, UINT unique);
#endif

extern void fgStartMenu(SFG_WinMenu * menu, SFG_Window * window, int x, int y, int x_win, int y_win);
extern void fgFinishMenu(SFG_Window * win, int x, int y);
extern void fgSetMenu(SFG_WinMenu * menu);

#if TARGET_HOST_WIN32
extern HMENU			fgHMenu;
#endif
extern unsigned			fgMenuButton;
extern SFG_WinMenu		*fgMappedMenu;
extern SFG_WinMenu		*fgCurrentMenu;
extern SFG_WinMenuItem	*fgItemSelected;
extern SFG_WinMenu		**fgMenuList;
extern SFG_WinMenu		*fgCurrentMenu;

#endif // USE_GLMENU

/*
 * Display the mouse cursor using OpenGL calls. The function
 * is defined in glutes_cursor.c file.
 */
void fgDisplayCursor( void );

/*
 * Elapsed time as per glutGet(GLUT_ELAPSED_TIME).
 */
long fgElapsedTime( void );

/*
 * List functions
 */
void fgListInit(SFG_List *list);
void fgListAppend(SFG_List *list, SFG_Node *node);
void fgListRemove(SFG_List *list, SFG_Node *node);
int fgListLength(SFG_List *list);
void fgListInsert(SFG_List *list, SFG_Node *next, SFG_Node *node);

/*
 * Error Messages functions
 */
void fgError( const char *fmt, ... );
void fgWarning( const char *fmt, ... );

#endif /* glutes_internal_H */

/*** END OF FILE ***/
