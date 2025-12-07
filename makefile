RAY_DEFS := -DPLATFORM_DESKTOP \
           -DGRAPHICS_API_OPENGL_33 \
           -DSUPPORT_X11 \
           -D_GLFW_X11

CC := gcc
#CFLAGS := -Wall -Wextra
LRAY =
LDFLAGS = $(LRAY) -lGL -lm -lpthread -ldl -lrt -lX11

RAY := thirdparty/raylib-5.5/src
GENERAL_HEADERS := includes/

SRC := src/**/*.c 

INCLUDE_RAY := -I$(RAY) \
           -I$(RAY)/external/glfw/include

SYS_RAY ?= 0

ifeq ($(SYS_RAY), 1)
	LRAY = -lraylib
	RAY_DEFS := 
	INCLUDE_RAY := 
else
	SRC += $(RAY)/*.c
endif

INCLUDES := -Ithirdparty \
	   -I$(GENERAL_HEADERS) \
	   $(INCLUDE_RAY)


all:
	$(CC) $(SRC) $(INCLUDES) $(RAY_DEFS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.out
