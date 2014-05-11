#
# Compile and link options
#

CXX=g++
CXXFLAGS=-Wall -I. -g -DUSE_JPEG

#stuff for irrKlang if it doesnt work
#install_name_tool -change @load_path/libirrklang.dylib /Users/vinay/Desktop/cos426-final/cos426-final/src tron
#  export DYLD_LIBRARY_PATH=/desktop/cos426-final/cos426-final/src
# ⚡ echo $DYLD_LIBRARY_PATH


# SELECT APPROPRIATE IRRKLANG LIBRARIES FOR DIFFERENT PLATFORMS                       
ifeq ($(OS),Windows_NT)                                                  
    IRRLIB =
    IRRLIBPATH =
    RMIRR =
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        IRRLIB = irrklang/libIrrKlang.so
        IRRLIBPATH = /usr/local/lib/
        RMIRR = /usr/local/lib/libIrrKlang.so
    endif
    ifeq ($(UNAME_S),Darwin)
        IRRLIB = irrKlang/libirrklang.dylib
        IRRLIBPATH = /usr/local/lib/
        RMIRR = /usr/local/lib/libirrklang.dylib
    endif
endif


#
# OpenGL libraries
#
UNAME := $(shell uname)
ifneq (,$(findstring Darwin,$(UNAME)))
	GLLIBS = -framework GLUT -framework OpenGL
        CXXFLAGS := $(CXXFLAGS) -mmacosx-version-min=10.8
else
  ifneq (,$(findstring CYGWIN,$(UNAME)))
	GLLIBS = -lopengl32 -lglu32 -lwinmm -lgdi32
  else
	GLLIBS = -lGLU -lGL -lX11
  endif
endif



#
# GNU Make: targets that don't build files
#

.PHONY: all clean distclean

#
# Rules encoding targets and dependencies.  By default, the first of
# these is built, but you can also build any individual target by
# passing it to make - e.g., "make imgpro" or "make clean"
#
# Notice that many of the dependencies are implicit (e.g. a .o depends
# on its corresponding .cpp), as are many of the compilation rules.
#

OBJS=tron.o game.o R3Scene.o
LIBS=R3/libR3.a R2/libR2.a jpeg/libjpeg.a fglut/libfglut.a
# IRR := libirrklang.dylib



all: tron

R3/libR3.a:
	    $(MAKE) -C R3

R2/libR2.a:
	    $(MAKE) -C R2

jpeg/libjpeg.a:
	    $(MAKE) -C jpeg

fglut/libfglut.a:
	$(MAKE) -C fglut

tron: $(OBJS) $(LIBS)
	    rm -f $@
	    $(CXX) $(CXXFLAGS) $^ -lm -o $@ $(GLLIBS) $(IRRLIB)

clean:
	    rm -f *.o tron
		$(MAKE) -C R3 clean
		$(MAKE) -C R2 clean
		$(MAKE) -C jpeg clean
		$(MAKE) -C fglut clean


distclean:  clean

setup:
	cp $(IRRLIB) $(IRRLIBPATH)

unsetup:
	rm -i $(RMIRR)

