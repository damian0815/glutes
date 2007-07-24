/*
 * glutes_font.c
 *
 * Bitmap and stroke fonts displaying.
 *
 * Copyright (c) 2005 Joachim Pouderoux. All Rights Reserved.
 * Written by Joachim Pouderoux <pouderou@labri.fr>
 * Creation date: Mon Jan 24 2005
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * the Software, and to permit persons to whom the Software is furnished 
 * to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * JOACHIM POUDEROUX BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Inc/GLES/glutes.h"
#include "glutes_internal.h"

extern SFG_Font fgFontFixed8x13;
extern SFG_Font fgFontFixed9x15;
extern SFG_Font fgFontHelvetica10;
extern SFG_Font fgFontHelvetica12;
extern SFG_Font fgFontHelvetica18;
extern SFG_Font fgFontTimesRoman10;
extern SFG_Font fgFontTimesRoman24;
extern SFG_StrokeFont fgStrokeRoman;
extern SFG_StrokeFont fgStrokeMonoRoman;


///////////////////////////////////////

/*
 * Matches a font ID with a SFG_Font structure pointer.
 * This was changed to match the GLUT header style.
 */
static SFG_Font* fghFontByID(void* font)
{
    if(font == GLUT_BITMAP_8_BY_13       )
        return &fgFontFixed8x13;
    if(font == GLUT_BITMAP_9_BY_15       )
        return &fgFontFixed9x15;
    if(font == GLUT_BITMAP_HELVETICA_10  )
        return &fgFontHelvetica10;
    if(font == GLUT_BITMAP_HELVETICA_12  )
        return &fgFontHelvetica12;
    if(font == GLUT_BITMAP_HELVETICA_18  )
        return &fgFontHelvetica18;
    if(font == GLUT_BITMAP_TIMES_ROMAN_10)
        return &fgFontTimesRoman10;
    if(font == GLUT_BITMAP_TIMES_ROMAN_24)
        return &fgFontTimesRoman24;
    return 0; /*** NOT REACHED ***/
}

/*
 * Matches a font ID with a SFG_StrokeFont structure pointer.
 * This was changed to match the GLUT header style.
 */
static SFG_StrokeFont* fghStrokeByID(void* font)
{
    if(font == GLUT_STROKE_ROMAN     )
        return &fgStrokeRoman;
    if(font == GLUT_STROKE_MONO_ROMAN)
        return &fgStrokeMonoRoman;

    return 0; /*** NOT REACHED ***/
}


void 
BitsToBytes(const GLubyte *bits, int ssx, int ssy, GLubyte *bytes, int dsx, int dsy)
{
	int ssxsy = ssx * ssy; // total bit #
	int x = 0;
	int bit = 7;

	bytes += ssy*dsx;
	
	while(ssxsy--)
	{
		bytes[x] = (bits[0] & (1 << bit)) ? 255 : 0;

		if(--bit == -1)
		{
			bit = 7;
			bits++;
		}
		if(++x >= ssx)
		{
			x = 0;
			bytes -= dsx; // retour à la ligne dans la destination
		}
	}
}

void 
BitsToShorts(const GLubyte *bits, int ssx, int ssy, GLushort *shorts, int dsx, int dsy)
{
	int ssxsy = ssx * ssy; // total bit #
	int x = 0;
	int bit = 7;

	shorts += ssy*dsx;
	
	while(ssxsy--)
	{
		shorts[x] = (bits[0] & (1 << bit)) ? 0xFFFF : 0x0000;

		if(--bit == -1)
		{
			bit = 7;
			bits++;
		}
		if(++x >= ssx)
		{
			x = 0;
			shorts -= dsx; // retour à la ligne dans la destination
		}
	}
}

int 
BitsToIndexedShorts(const GLubyte *bits, int ssx, int ssy, GLshort *points, int startx, int starty)
{
	int ssxsy = ssx * ssy; // total bit #
	int x = 0;
	int bit = 7, count = 0;

	while(ssxsy--)
	{
		if(bits[0] & (1 << bit))
		{
			points[0] = startx + x;
			points[1] = starty;
			count++;	
			points += 2;
		}
		if(--bit == -1)
		{
			bit = 7;
			bits++;
		}
		if(++x >= ssx)
		{
			x = 0;
			starty++;
		}
	}
	return count;
}

/*
 * Draw a bitmap character
 */
void 
__glutBitmapCharacter(SFG_Font* font, int character)
{
	int i, nbpoints;
	GLshort *points;
	const GLubyte *face;
	GLushort indices[64*64];

    if(!(character >= 1)&&(character < 256))
		return;

    /*
     * Find the character we want to draw (???)
     */
    face = font->Characters[ character - 1 ];

	points = (GLshort*)malloc(font->Height * face[0] * 2 * sizeof(GLshort));
	nbpoints = BitsToIndexedShorts(face+1, face[0], font->Height, points, 0, 0);

	glEnable(GL_ALPHA_TEST);
    glAlphaFuncx(GL_NOTEQUAL, 0);

	for(i = 0; i < nbpoints; i++)
		indices[i] = i;

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_SHORT, 0, points);

	glDrawElements(GL_POINTS,
                        nbpoints,
                        GL_UNSIGNED_SHORT,
                        indices);

	glDisable(GL_ALPHA_TEST);
	free(points);
}


void FGAPIENTRY 
glutBitmapCharacterPoints(void* fontID, int x, int y, int character)
{
	const GLubyte* face;
	SFG_Font* font = fghFontByID(fontID);

	if(!font)
		return;

    if(!(character >= 1)&&(character < 256))
		return;

    face = font->Characters[ character - 1 ];

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); 

	glOrthox(0, _INT2FIXED(fgStructure.Window->State.Width), 0, 
		_INT2FIXED(fgStructure.Window->State.Height), 0, _INT2FIXED(1));
	
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity(); 

	glDisable(GL_DEPTH_TEST);
	
	glTranslatex(_INT2FIXED(x), _INT2FIXED(y), 0);
	__glutBitmapCharacter(font, character);
	glTranslatex(_INT2FIXED(face[0]), 0, 0);

	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void FGAPIENTRY 
glutBitmapStringPoints(void* fontID, int x, int y, const char *string)
{
    SFG_Font* font = fghFontByID(fontID);
	int numchar, xx = 0, yy = 0, nbpoints = 0, i, c;
	GLshort *points;
	GLushort *indices;

	if(!font || !string)
		return;
	if(!string[0])
		return;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); 

	glOrthox(0, _INT2FIXED(fgStructure.Window->State.Width), 0, 
		_INT2FIXED(fgStructure.Window->State.Height), 0, _INT2FIXED(1));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity(); 

	glDisable(GL_DEPTH_TEST);
	
	glTranslatex(_INT2FIXED(x), _INT2FIXED(y), 0);

	numchar = (int)strlen((char*) string);

	points = (GLshort*)malloc(numchar * 
		font->Height * (*(font->Characters[ 'X' - 1 ])) *
		8 * sizeof(GLshort));

    for(c = 0; c < numchar; c++)
    {
		if(string[c] == '\n')
		{
			yy -= font->Height;
			xx = 0;
		}
		else
		{
			const GLubyte* face = font->Characters[ string[c] - 1 ];
			nbpoints += BitsToIndexedShorts(face+1, face[0], font->Height, points+(nbpoints*2), xx, yy);
			xx += face[0];
		}
	}

	glEnable(GL_ALPHA_TEST);
    glAlphaFuncx(GL_NOTEQUAL, 0);

	indices = (GLushort*)malloc(nbpoints*sizeof(GLushort));
	for(i = 0; i < nbpoints; i++)
		indices[i] = i;

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_SHORT, 0, points);

	glDrawElements(GL_POINTS,
                        nbpoints,
                        GL_UNSIGNED_SHORT,
                        indices);

	glDisable(GL_ALPHA_TEST);
	free(indices);
	free(points);

	glEnable(GL_DEPTH_TEST);
	
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}


#if TARGET_HOST_WIN32
void FGAPIENTRY
glutTrueTypeStringPoints(WCHAR *fontname, int fontsize, int style, int x, int y, const WCHAR *string)
{
	int len, xx = 0, yy = 0, nbpoints = 0, i;
	GLshort *points;
	GLushort *indices;
	HFONT font;
	LOGFONTW	lf;	
	RECT rect;
	static HBITMAP bmp;
	static BYTE *img;
	static HDC hdc = NULL;
	static BITMAPINFO bi;
	SIZE sz;
	static EGLint width, height;

	if(!fontname || !string)
		return;
	if(!string[0])
		return;

	// Initialize static DC and DIB bitmap on the first call
	if(!hdc)
	{		
		// Create a device compatible DC
		hdc = CreateCompatibleDC(GetDC(fgStructure.Window->Window.Handle));	
		
		eglQuerySurface(fgDisplay.eglDisplay, fgStructure.Window->Window.Surface, EGL_WIDTH, &width);
		eglQuerySurface(fgDisplay.eglDisplay, fgStructure.Window->Window.Surface, EGL_HEIGHT, &height);

		// Create a DIB bitmap and attach it to the DC
		ZeroMemory(&bi, sizeof(BITMAPINFO));
		bi.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
		bi.bmiHeader.biWidth = width;
		bi.bmiHeader.biHeight = height; 
		bi.bmiHeader.biBitCount = 8;
		bi.bmiHeader.biPlanes = 1;
		bi.bmiHeader.biCompression = BI_RGB;
		
		bmp = CreateDIBSection(hdc, &bi, DIB_RGB_COLORS, &img, NULL, 0);
	
		SelectObject(hdc, bmp);
		
		SelectObject(hdc, GetStockObject(BLACK_BRUSH));

		SetBkMode(hdc, TRANSPARENT);
		SetTextColor(hdc, RGB(255, 255, 255));			
	}

	// Erase DC content	with the current black brush
	//Rectangle(hdc, 0, 0, width, height);
	ZeroMemory(img, width * height);

	// Create the font handle and attach it to the DC
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfEscapement = 0;
	wcscpy(lf.lfFaceName, fontname);
	lf.lfHeight = -(fontsize * GetDeviceCaps(GetDC(fgStructure.Window->Window.Handle), LOGPIXELSY) / 72);
	lf.lfItalic = (style & 1) ? TRUE : FALSE;
	lf.lfOrientation = 0;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfStrikeOut = FALSE;
	lf.lfUnderline = (style & 4) ? TRUE : FALSE;
	lf.lfWidth = 0;
	lf.lfWeight = (style & 2) ? FW_BOLD : FW_NORMAL;
	
	font = CreateFontIndirectW(&lf);

	SelectObject(hdc, font);

	// Draw text in white onto the bitmap
	len = wcslen(string);

	GetTextExtentPointW(hdc, string, len, &sz);

	rect.left = max(0, min(x, width));
	rect.top = max(0, min(y, height));
	rect.right = min(rect.left + sz.cx, width);
	rect.bottom = min(rect.top + sz.cy, height);

	DrawTextW(hdc, string, len, &rect, DT_LEFT | DT_BOTTOM);
	
	// Traverse the bitmap and add all white pixels into a points buffer
	points = (GLshort*)malloc(sz.cx * sz.cy * 2 * sizeof(short));

	for(yy = rect.top; yy < rect.bottom; yy++)
	{
		for(xx = rect.left; xx < rect.right; xx++)
		{
			if(img[xx + (height - yy) * width] != 0)
			{
				points[nbpoints * 2 + 0] = xx - x;
				points[nbpoints * 2 + 1] = (short)(rect.top + sz.cy - (yy - rect.top)) - y;
				nbpoints++;
			}
		}
	}

	// Delete GDI font object
	DeleteObject(font);
	
	// Prepare the index buffer
	indices = (GLushort*)malloc(nbpoints * sizeof(GLushort));
	for(i = 0; i < nbpoints; i++)
		indices[i] = i;

	// Draw the points buffer
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity(); 

	glOrthox(0, _INT2FIXED(width), 
			0, _INT2FIXED(height), 
			0, _INT2FIXED(1));


	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity(); 

	glDisable(GL_DEPTH_TEST);
	
	glTranslatex(_INT2FIXED(x), _INT2FIXED(y), 0);	
	
	glEnable(GL_ALPHA_TEST);
    glAlphaFuncx(GL_NOTEQUAL, 0);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_SHORT, 0, points);

	glDrawElements(GL_POINTS,
                        nbpoints,
                        GL_UNSIGNED_SHORT,
                        indices);
	
	glDisable(GL_ALPHA_TEST);

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);

	free(indices);
	free(points);
}
#endif /* TARGET_HOST_WIN32 */


#ifdef GL_OES_draw_texture
/*
 * Draw a bitmap character
 */
void FGAPIENTRY 
glutBitmapCharacterTex(void* fontID, int x, int y, int character)
{
#if (GL_OES_VERSION_1_1 >= 1)
    const GLubyte* face;
    SFG_Font* font = fghFontByID(fontID);
	GLint v[] = {0, 0, _INT2FIXED(64), _INT2FIXED(64)}; //face[0], font->Height};
	GLubyte buff[64*64];
	GLuint tid;

	if(!font)
		return;

    if(!(character >= 1)&&(character < 256))
		return;

    /*
     * Find the character we want to draw (???)
     */
    face = font->Characters[ character - 1 ];
	
	memset(buff, 0, 64*64*sizeof(GLubyte));
	BitsToBytes(face+1, face[0], font->Height, buff, 64, 64);

	glGenTextures(1, &tid);
    glBindTexture(GL_TEXTURE_2D, tid);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
                            64, 64, 0, GL_ALPHA, GL_UNSIGNED_BYTE,
                            buff);

    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glEnable(GL_TEXTURE_2D);

    glTexEnvx(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEnable(GL_ALPHA_TEST);
    glAlphaFuncx(GL_EQUAL, 0);

	glTexParameterxv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, v);

	glDrawTexxOES(_INT2FIXED(x), _INT2FIXED(y), _INT2FIXED(0), _INT2FIXED(16), _INT2FIXED(16)); //face[0], font->Height);

    glDisable(GL_ALPHA_TEST);
    glDisable(GL_TEXTURE_2D);

	glDeleteTextures(1, &tid);
#endif
}

void FGAPIENTRY 
glutBitmapStringTex(void* fontID, int x, int y, const char *string)
{
    SFG_Font* font = fghFontByID(fontID);
	int xx = x;
	int c, numchar = strlen((char*)string);
	
	if(!font)
		return;

	for(c = 0; c < numchar; c++)
	{
		if(string[c] == '\n')
		{
			xx = x;
			y -= font->Height;
		} 
		else
		{
			glutBitmapCharacterTex(fontID, xx, y, string[c]);
			xx += *(font->Characters[ string[c] - 1 ]);
		}
	}
}

#endif

/*
 * Returns the width in pixels of a font's character
 */
int FGAPIENTRY 
glutBitmapWidth(void* fontID, int character)
{
    SFG_Font* font = fghFontByID(fontID);

    if(character > 0 && character < 256)
		return *(font->Characters[ character - 1 ]);
	return 0;
}

/*
 * Return the width of a string drawn using a bitmap font
 */
int FGAPIENTRY 
glutBitmapLength(void* fontID, const char* string)
{
    int c, length = 0, this_line_length = 0;
    SFG_Font* font = fghFontByID(fontID);
    int numchar = (int)strlen((char*) string);

    for(c = 0; c < numchar; c++)
    {
        if(string[ c ] != '\n')/* Not an EOL, increment length of line */
            this_line_length += *(font->Characters[ string[ c ] - 1 ]);
        else  /* EOL; reset the length of this line */
        {
            if(length < this_line_length)
                length = this_line_length;
            this_line_length = 0;
        }
    }
    if (length < this_line_length)
        length = this_line_length;

    return length;
}

/*
 * Returns the height of a bitmap font
 */
int FGAPIENTRY 
glutBitmapHeight(void* fontID)
{
    SFG_Font* font = fghFontByID(fontID);
    return font->Height;
}


/*
 * Draw a stroke character
 */
void FGAPIENTRY 
glutStrokeCharacter(void* fontID, int character)
{
    const SFG_StrokeChar *schar;
    const SFG_StrokeStrip *strip;
    int i;
    SFG_StrokeFont* font = fghStrokeByID(fontID);
	GLbyte indices[255];

    if(! (character >= 0) && (character < font->Quantity))
		return;

    schar = font->Characters[ character ];
    if(! schar)
		return;
    strip = schar->Strips;
	
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);

	
	for(i = 0; i < 255; i++)
		indices[i] = i;

    for(i = 0; i < schar->Number; i++, strip++)
    {
		glVertexPointer(2, GL_FIXED, 0, strip->Vertices);
		glDrawElements(GL_LINE_STRIP,
                        strip->Number,
                        GL_UNSIGNED_BYTE,
                        indices);
    }
    glTranslatex((GLfixed)schar->Right, 0, 0);
}

void FGAPIENTRY 
glutStrokeString(void* fontID, const char *string)
{
    int c, i;
    int numchar = (int)strlen((char*) string);
    GLfixed length = 0;
	GLbyte indices[255];
    SFG_StrokeFont* font = fghStrokeByID(fontID);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
	glEnableClientState (GL_VERTEX_ARRAY);

	for(i = 0; i < 255; i++)
		indices[i] = i;

    /*
     * Step through the string, drawing each character.
     * A newline will simply translate the next character's insertion
     * point back to the start of the line and down one line.
     */
    for(c = 0; c < numchar; c++)
        if(string[ c ] < font->Quantity)
        {
            if(string[ c ] == '\n')
            {
                glTranslatex(-length, -font->Height, 0);
                length = 0;
            }
            else  /* Not an EOL, draw the bitmap character */
            {
                const SFG_StrokeChar *schar = font->Characters[ string[ c ] ];
                if(schar)
                {
                    const SFG_StrokeStrip *strip = schar->Strips;

                    for(i = 0; i < schar->Number; i++, strip++)
                    {
						glVertexPointer(2, GL_FIXED, 0, strip->Vertices);
						glDrawElements(GL_LINE_STRIP,
							strip->Number,
							GL_UNSIGNED_BYTE,
							indices);
                    }
                    
                    length += schar->Right;
                    glTranslatex(schar->Right, 0, 0);
                }
            }
        }
}

/*
 * Return the width in pixels of a stroke character
 */
int FGAPIENTRY 
glutStrokeWidth(void* fontID, int character)
{
    const SFG_StrokeChar *schar;
    SFG_StrokeFont* font = fghStrokeByID(fontID);

    if(! ((character >= 0) && (character < font->Quantity)))
		return 0;
    schar = font->Characters[ character ];
    if(!schar)
		return 0;
    
    return (int)(schar->Right / 65536);
}

/*
 * Return the width of a string drawn using a stroke font
 */
int FGAPIENTRY 
glutStrokeLength(void* fontID, const char* string)
{
    int c;
    GLfixed length = 0;
    GLfixed this_line_length = 0;
    SFG_StrokeFont* font = fghStrokeByID(fontID);
    int numchar = (int)strlen((char*) string);

    for(c = 0; c < numchar; c++)
        if(string[ c ] < font->Quantity)
        {
            if(string[ c ] == '\n') /* EOL; reset the length of this line */
            {
                if(length < this_line_length)
                    length = this_line_length;
                this_line_length = 0;
            }
            else  /* Not an EOL, increment the length of this line */
            {
                const SFG_StrokeChar *schar = font->Characters[ string[ c ] ];
                if(schar)
                    this_line_length += schar->Right;
            }
        }

    if(length < this_line_length)
        length = this_line_length;
    return (int)(length / 65536);
}

/*
 * Returns the height of a stroke font
 */
GLfloat FGAPIENTRY 
glutStrokeHeight(void* fontID)
{
    SFG_StrokeFont* font = fghStrokeByID(fontID);
    return _FIXED2FLOAT(font->Height);
}


/*** END OF FILE ***/
