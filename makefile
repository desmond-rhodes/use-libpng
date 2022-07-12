OUT := display.exe
OBJS := main.o display.o lib/gl3w.o
DEPS := $(OBJS:%.o=%.d)

CC := x86_64-w64-mingw32-gcc-posix
CFLAGS := -std=c17

CXX := x86_64-w64-mingw32-g++-posix
CXXFLAGS := -std=c++17 -Wall -Wextra

CPPFLAGS := -Iinclude

TARGET_ARCH := -Og

LDFLAGS := -Llib
LDLIBS := -lglfw3 -lz -lpng -lgdi32 -static-libgcc -static-libstdc++ -static -lpthread

DEPFLAGS = -MT $@ -MMD -MP -MF $*.d

$(OUT): $(OBJS)
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

$(OBJS): | .setup

%.o: %.c
	$(COMPILE.c) $(DEPFLAGS) $(OUTPUT_OPTION) $<

%.o: %.cc
	$(COMPILE.cc) $(DEPFLAGS) $(OUTPUT_OPTION) $<

.PHONY: clean
clean:
	rm -f *.d *.o $(OUT)

include $(wildcard $(DEPS))

.setup: glfw gl3w zlib libpng
	cd gl3w && python3 gl3w_gen.py
	cmake -S glfw -B glfw/build -D CMAKE_TOOLCHAIN_FILE=CMake/x86_64-w64-mingw32.cmake
	make -C glfw/build glfw --no-print-directory
	cd zlib && ./configure --static
	make -C zlib libz.a --no-print-directory
	touch libpng/pnglibconf.dfn
	cp libpng/scripts/makefile.std libpng/makefile
	cp libpng/scripts/pnglibconf.h.prebuilt libpng/pnglibconf.h
	make -C libpng libpng.a --no-print-directory
	mkdir -p include include/GLFW include/GL include/KHR lib
	ln -sf ../../glfw/include/GLFW/glfw3.h include/GLFW/glfw3.h
	ln -sf ../../gl3w/include/GL/gl3w.h include/GL/gl3w.h
	ln -sf ../../gl3w/include/GL/glcorearb.h include/GL/glcorearb.h
	ln -sf ../../gl3w/include/KHR/khrplatform.h include/KHR/khrplatform.h
	ln -sf ../zlib/zlib.h include/zlib.h
	ln -sf ../zlib/zconf.h include/zconf.h
	ln -sf ../libpng/png.h include/png.h
	ln -sf ../libpng/pnglibconf.h include/pnglibconf.h
	ln -sf ../libpng/pngconf.h include/pngconf.h
	ln -sf ../glfw/build/src/libglfw3.a lib/libglfw3.a
	ln -sf ../gl3w/src/gl3w.c lib/gl3w.c
	ln -sf ../zlib/libz.a lib/libz.a
	ln -sf ../libpng/libpng.a lib/libpng.a
	touch .setup

.PHONY: reset-everything
reset-everything:
	-make -C zlib clean --no-print-directory
	cd zlib && git reset --hard && git clean --force
	-make -C libpng clean --no-print-directory
	cd libpng && git reset --hard && git clean --force
	rm -rf include lib glfw/build gl3w/include
	rm -f gl3w/src/gl3w.c
	rm -f $(DEPS) $(OBJS) $(OUT) .setup
