INCLUDES=-I../../Inc

includedir = ${prefix}/include/GLES/
include_HEADERS=../../Inc/GLES/glutes.h
lib_LTLIBRARIES=libglutes.la

libglutes_la_LIBADD=-lGLES_CM -lX11
libglutes_la_LDFLAGS=-version-info 1:0:0
libglutes_la_SOURCES=../../Src/glutes_callbacks.c \
                    ../../Src/glutes_cursor.c \
                    ../../Src/glutes_display.c \
                    ../../Src/glutes_ext.c \
                    ../../Src/glutes_font.c \
                    ../../Src/glutes_font_data.c \
                    ../../Src/glutes_gamemode.c \
                    ../../Src/glutes_glutfont_definitions.c \
                    ../../Src/glutes_init.c \
                    ../../Src/glutes_joystick.c \
                    ../../Src/glutes_main.c \
                    ../../Src/glutes_menu.c \
                    ../../Src/glutes_menugl.c \
                    ../../Src/glutes_misc.c \
                    ../../Src/glutes_overlay.c \
                    ../../Src/glutes_shape.c \
                    ../../Src/glutes_shapex.c \
                    ../../Src/glutes_state.c \
                    ../../Src/glutes_stroke_mono_roman_fixed.c \
                    ../../Src/glutes_stroke_roman_fixed.c \
                    ../../Src/glutes_structure.c \
                    ../../Src/glutes_videoresize.c \
                    ../../Src/glutes_window.c

