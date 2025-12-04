RAY_DEFS = -DPLATFORM_DESKTOP \
           -DGRAPHICS_API_OPENGL_33 \
           -DSUPPORT_X11 \
           -D_GLFW_X11

CC = gcc
#CFLAGS = -Wall -Wextra
LDFLAGS = -lGL -lm -lpthread -ldl -lrt -lX11

RAY = thirdparty/raylib-5.5/src
SRC = src/*.c $(RAY)/*.c

INCLUDES = -Ithirdparty \
           -I$(RAY) \
           -I$(RAY)/external/glfw/include

all:
	$(CC) $(SRC) $(INCLUDES) $(RAY_DEFS) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f *.out
