/*
 * glutes_ext.c
 *
 * Functions related to OpenGL extensions.
 *
 * Copyright (c) 2005 Joachim Pouderoux. All Rights Reserved.
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
 * THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define GLX_GLXEXT_PROTOTYPES

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"

#ifdef _WIN32
#	include <commdlg.h>
#endif


struct name_address_pair
{
    const char *name;
    void *address;
};

static struct name_address_pair glut_functions[] =
{
   { "glutInit", (void *) glutInit },
   { "glutInitDisplayMode", (void *) glutInitDisplayMode },
   { "glutInitDisplayString", (void *) glutInitDisplayString },
   { "glutInitWindowPosition", (void *) glutInitWindowPosition },
   { "glutInitWindowSize", (void *) glutInitWindowSize },
   { "glutMainLoop", (void *) glutMainLoop },
   { "glutCreateWindow", (void *) glutCreateWindow },
   { "glutCreateSubWindow", (void *) glutCreateSubWindow },
   { "glutDestroyWindow", (void *) glutDestroyWindow },
   { "glutPostRedisplay", (void *) glutPostRedisplay },
   { "glutPostWindowRedisplay", (void *) glutPostWindowRedisplay },
   { "glutSwapBuffers", (void *) glutSwapBuffers },
   { "glutGetWindow", (void *) glutGetWindow },
   { "glutSetWindow", (void *) glutSetWindow },
   { "glutSetWindowTitle", (void *) glutSetWindowTitle },
   { "glutSetIconTitle", (void *) glutSetIconTitle },
   { "glutPositionWindow", (void *) glutPositionWindow },
   { "glutReshapeWindow", (void *) glutReshapeWindow },
   { "glutPopWindow", (void *) glutPopWindow },
   { "glutPushWindow", (void *) glutPushWindow },
   { "glutIconifyWindow", (void *) glutIconifyWindow },
   { "glutShowWindow", (void *) glutShowWindow },
   { "glutHideWindow", (void *) glutHideWindow },
   { "glutFullScreen", (void *) glutFullScreen },
   { "glutSetCursor", (void *) glutSetCursor },
   { "glutWarpPointer", (void *) glutWarpPointer },
   { "glutEstablishOverlay", (void *) glutEstablishOverlay },
   { "glutRemoveOverlay", (void *) glutRemoveOverlay },
   { "glutUseLayer", (void *) glutUseLayer },
   { "glutPostOverlayRedisplay", (void *) glutPostOverlayRedisplay },
   { "glutPostWindowOverlayRedisplay", (void *) glutPostWindowOverlayRedisplay },
   { "glutShowOverlay", (void *) glutShowOverlay },
   { "glutHideOverlay", (void *) glutHideOverlay },
   { "glutCreateMenu", (void *) glutCreateMenu },
   { "glutDestroyMenu", (void *) glutDestroyMenu },
   { "glutGetMenu", (void *) glutGetMenu },
   { "glutSetMenu", (void *) glutSetMenu },
   { "glutAddMenuEntry", (void *) glutAddMenuEntry },
   { "glutAddSubMenu", (void *) glutAddSubMenu },
   { "glutChangeToMenuEntry", (void *) glutChangeToMenuEntry },
   { "glutChangeToSubMenu", (void *) glutChangeToSubMenu },
   { "glutRemoveMenuItem", (void *) glutRemoveMenuItem },
   { "glutAttachMenu", (void *) glutAttachMenu },
   { "glutDetachMenu", (void *) glutDetachMenu },
   { "glutDisplayFunc", (void *) glutDisplayFunc },
   { "glutReshapeFunc", (void *) glutReshapeFunc },
   { "glutKeyboardFunc", (void *) glutKeyboardFunc },
   { "glutMouseFunc", (void *) glutMouseFunc },
   { "glutMotionFunc", (void *) glutMotionFunc },
   { "glutPassiveMotionFunc", (void *) glutPassiveMotionFunc },
   { "glutEntryFunc", (void *) glutEntryFunc },
   { "glutVisibilityFunc", (void *) glutVisibilityFunc },
   { "glutIdleFunc", (void *) glutIdleFunc },
   { "glutTimerFunc", (void *) glutTimerFunc },
   { "glutMenuStateFunc", (void *) glutMenuStateFunc },
   { "glutSpecialFunc", (void *) glutSpecialFunc },
   { "glutSpaceballMotionFunc", (void *) glutSpaceballMotionFunc },
   { "glutSpaceballRotateFunc", (void *) glutSpaceballRotateFunc },
   { "glutSpaceballButtonFunc", (void *) glutSpaceballButtonFunc },
   { "glutButtonBoxFunc", (void *) glutButtonBoxFunc },
   { "glutDialsFunc", (void *) glutDialsFunc },
   { "glutTabletMotionFunc", (void *) glutTabletMotionFunc },
   { "glutTabletButtonFunc", (void *) glutTabletButtonFunc },
   { "glutMenuStatusFunc", (void *) glutMenuStatusFunc },
   { "glutOverlayDisplayFunc", (void *) glutOverlayDisplayFunc },
   { "glutWindowStatusFunc", (void *) glutWindowStatusFunc },
   { "glutKeyboardUpFunc", (void *) glutKeyboardUpFunc },
   { "glutSpecialUpFunc", (void *) glutSpecialUpFunc },
   { "glutJoystickFunc", (void *) glutJoystickFunc },
   { "glutSetColor", (void *) glutSetColor },
   { "glutGetColor", (void *) glutGetColor },
   { "glutCopyColormap", (void *) glutCopyColormap },
   { "glutGet", (void *) glutGet },
   { "glutDeviceGet", (void *) glutDeviceGet },
   { "glutExtensionSupported", (void *) glutExtensionSupported },
   { "glutGetModifiers", (void *) glutGetModifiers },
   { "glutLayerGet", (void *) glutLayerGet },
   { "glutBitmapCharacter", (void *) glutBitmapCharacterPoints },
   { "glutBitmapWidth", (void *) glutBitmapWidth },
   { "glutStrokeCharacter", (void *) glutStrokeCharacter },
   { "glutStrokeWidth", (void *) glutStrokeWidth },
   { "glutBitmapLength", (void *) glutBitmapLength },
   { "glutStrokeLength", (void *) glutStrokeLength },
#ifndef GLES_CL
   { "glutWireSphere", (void *) glutWireSphere },
   { "glutSolidSphere", (void *) glutSolidSphere },
//   { "glutWireCone", (void *) glutWireCone },
   { "glutSolidCone", (void *) glutSolidCone },
   { "glutWireCube", (void *) glutWireCube },
   { "glutSolidCube", (void *) glutSolidCube },
//   { "glutWireTorus", (void *) glutWireTorus },
   { "glutSolidTorus", (void *) glutSolidTorus },
//   { "glutWireDodecahedron", (void *) glutWireDodecahedron },
//   { "glutSolidDodecahedron", (void *) glutSolidDodecahedron },
//   { "glutWireTeapot", (void *) glutWireTeapot },
//   { "glutSolidTeapot", (void *) glutSolidTeapot },
//   { "glutWireOctahedron", (void *) glutWireOctahedron },
//   { "glutSolidOctahedron", (void *) glutSolidOctahedron },
//   { "glutWireTetrahedron", (void *) glutWireTetrahedron },
//   { "glutSolidTetrahedron", (void *) glutSolidTetrahedron },
//   { "glutWireIcosahedron", (void *) glutWireIcosahedron },
//   { "glutSolidIcosahedron", (void *) glutSolidIcosahedron },
#endif
   { "glutWireSpherex", (void *) glutWireSpherex },
   { "glutSolidSpherex", (void *) glutSolidSpherex },
//   { "glutWireConex", (void *) glutWireConex },
   { "glutSolidConex", (void *) glutSolidConex },
   { "glutWireCubex", (void *) glutWireCubex },
   { "glutSolidCubex", (void *) glutSolidCubex },
//   { "glutWireTorusx", (void *) glutWireTorusx },
   { "glutSolidTorusx", (void *) glutSolidTorusx },
//   { "glutWireDodecahedronx", (void *) glutWireDodecahedronx },
//   { "glutSolidDodecahedronx", (void *) glutSolidDodecahedronx },
//   { "glutWireTeapotx", (void *) glutWireTeapotx },
//   { "glutSolidTeapotx", (void *) glutSolidTeapotx },
//   { "glutWireOctahedronx", (void *) glutWireOctahedronx },
//   { "glutSolidOctahedronx", (void *) glutSolidOctahedronx },
//   { "glutWireTetrahedronx", (void *) glutWireTetrahedronx },
//   { "glutSolidTetrahedronx", (void *) glutSolidTetrahedronx },
//   { "glutWireIcosahedronx", (void *) glutWireIcosahedronx },
//   { "glutSolidIcosahedronx", (void *) glutSolidIcosahedronx },
   { "glutVideoResizeGet", (void *) glutVideoResizeGet },
   { "glutSetupVideoResizing", (void *) glutSetupVideoResizing },
   { "glutStopVideoResizing", (void *) glutStopVideoResizing },
   { "glutVideoResize", (void *) glutVideoResize },
   { "glutVideoPan", (void *) glutVideoPan },
   { "glutReportErrors", (void *) glutReportErrors },
   { "glutIgnoreKeyRepeat", (void *) glutIgnoreKeyRepeat },
   { "glutSetKeyRepeat", (void *) glutSetKeyRepeat },
   { "glutForceJoystickFunc", (void *) glutForceJoystickFunc },
   { "glutGameModeString", (void *) glutGameModeString },
   { "glutEnterGameMode", (void *) glutEnterGameMode },
   { "glutLeaveGameMode", (void *) glutLeaveGameMode },
   { "glutGameModeGet", (void *) glutGameModeGet },
   /* freeglut extensions */
   { "glutMainLoopEvent", (void *) glutMainLoopEvent },
   { "glutLeaveMainLoop", (void *) glutLeaveMainLoop },
   { "glutCloseFunc", (void *) glutCloseFunc },
   { "glutWMCloseFunc", (void *) glutWMCloseFunc },
   { "glutMenuDestroyFunc", (void *) glutMenuDestroyFunc },
   { "glutSetOption", (void *) glutSetOption },
   { "glutSetWindowData", (void *) glutSetWindowData },
   { "glutGetWindowData", (void *) glutGetWindowData },
   { "glutSetMenuData", (void *) glutSetMenuData },
   { "glutGetMenuData", (void *) glutGetMenuData },
   { "glutBitmapHeight", (void *) glutBitmapHeight },
   { "glutStrokeHeight", (void *) glutStrokeHeight },
   { "glutBitmapString", (void *) glutBitmapStringPoints },
   { "glutStrokeString", (void *) glutStrokeString },
#if TARGET_HOST_WIN32
   { "glutTrueTypeString", (void *) glutTrueTypeString },
#endif
//   { "glutWireRhombicDodecahedron", (void *) glutWireRhombicDodecahedron },
//   { "glutSolidRhombicDodecahedron", (void *) glutSolidRhombicDodecahedron },
//   { "glutWireSierpinskiSponge ", (void *) glutWireSierpinskiSponge },
//   { "glutSolidSierpinskiSponge ", (void *) glutSolidSierpinskiSponge },
   { "glutGetProcAddress", (void *) glutGetProcAddress },
   { "glutMouseWheelFunc", (void *) glutMouseWheelFunc },
   { NULL, NULL }
};   


void *FGAPIENTRY glutGetProcAddress( const char *procName )
{
    /* Try GLUT functions first */
    int i;
    for( i = 0; glut_functions[ i ].name; i++ )
        if( strcmp( glut_functions[ i ].name, procName ) == 0)
            return glut_functions[ i ].address;

#if 0
    /* Try core GL functions */
#if TARGET_HOST_WIN32
    return( void * )wglGetProcAddress( ( LPCSTR )procName );
#elif TARGET_HOST_UNIX_X11 && defined( GLX_ARB_get_proc_address )
    return(void * )glXGetProcAddressARB( ( const GLubyte * )procName );
#endif
#endif
    return NULL;
}

int FGAPIENTRY glutSelectFile(char *filename, const char *filter, const char *title)
{
#ifdef _WIN32

	OPENFILENAME ofn;
#ifdef UNICODE
	int i = 0;
	WCHAR _filename[512];
	WCHAR _title[512];
	WCHAR _filter[512];
	mbstowcs(_filename, filename, strlen(filename) + 1);
	mbstowcs(_title, title, strlen(title) + 1);
	// Convert the multi-string filter
	while(i != 512)
	{
		if(filter[i] == 0 && filter[i+1] == 0)
		{
			_filter[i] = 0;
			_filter[i+1] = 0;
			break;
		}
		_filter[i] = filter[i];
		i++;
	}
	
#else
	char *_filename = (char*)filename;
	char *_title = (char*)title;
	char *_filter = (char*)filter;
#endif		
	//glutHideWindow();
	memset(&ofn, 0, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL; //__glutCurrentWindow->win;
	ofn.lpstrFilter = _filter;
	ofn.lpstrFile = _filename;
	ofn.nMaxFile = 512;
	ofn.lpstrTitle = _title;
	ofn.Flags = OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
	{	
#ifdef UNICODE
		wcstombs(filename, _filename, (size_t)512);
#endif
		glutShowWindow();
		return 1;
	}
	glutShowWindow();

#endif
	return 0;
}

