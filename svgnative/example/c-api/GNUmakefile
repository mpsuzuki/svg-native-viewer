PROJECT_TOPDIR = ../../../
SVG_NATIVE_TOPDIR = ${PROJECT_TOPDIR}/svgnative/
VIEWER_SRCDIR = ${SVG_NATIVE_TOPDIR}/src/
VIEWER_INCLUDEDIR = ${SVG_NATIVE_TOPDIR}/include/

PORTS_STRING_SRCDIR = ${SVG_NATIVE_TOPDIR}/ports/string/
PORTS_SKIA_SRCDIR = ${SVG_NATIVE_TOPDIR}/ports/skia/
PORTS_COREGRAPHICS_SRCDIR = ${SVG_NATIVE_TOPDIR}/ports/cg/
PORTS_CAIRO_SRCDIR = ${SVG_NATIVE_TOPDIR}/ports/cairo/
PORTS_QT_SRCDIR = ${SVG_NATIVE_TOPDIR}/ports/qt/

PORTS_CPPFLAGS = -I${VIEWER_INCLUDEDIR} \
		-I${VIEWER_SRCDIR} \
		-I${PORTS_STRING_SRCDIR} \
		-I${PORTS_SKIA_SRCDIR} \
		-I${PORTS_COREGRAPHICS_SRCDIR} \
		-I${PORTS_CAIRO_SRCDIR} \
		-I${PORTS_QT_SRCDIR}

SKIA_CPPFLAGS = -I${PROJECT_TOPDIR}/third_party/skia/include/config \
		-I${PROJECT_TOPDIR}/third_party/skia/include/core \
		-I${PROJECT_TOPDIR}/third_party/skia/include/effects \
		-I${PROJECT_TOPDIR}/third_party/skia/include/encode \
		-I${PROJECT_TOPDIR}/third_party/skia/include/gpu \
		-I${PROJECT_TOPDIR}/third_party/cpp-base64
CAIRO_CPPFLAGS = `pkg-config --cflags cairo`
CAIRO_LIBS = `pkg-config --libs cairo`
QT_CPPFLAGS = -I/usr/include/qt4 -I/usr/include/qt4/QtGui/ -I/usr/include/qt4 -I/usr/include/qt4/QtCore/

CXXFLAGS = -ggdb -g3 -O0 ${PORTS_CPPFLAGS} ${SKIA_CPPFLAGS} ${CAIRO_CPPFLAGS} ${QT_CPPFLAGS} -I.
CFLAGS = -ggdb -g3 -O0 -I.

OBJS = SVGNativeCAPI.o testCAPI.o

testCAPI.exe: ${OBJS}
	${CC} ${CFLAGS} -o $@ ${OBJS} -Wl,-rpath=`pwd` -L. -lSVGNativeViewerLib ${CAIRO_LIBS} -lstdc++

clean:
	rm -f ${OBJS}

SVGNativeCAPI.o: SVGNativeCAPI.cpp
	${CXX} ${CXXFLAGS} \
		-o $@ -c $<

testCAPI.o: testCAPI.c
	${CC} ${CFLAGS} ${CAIRO_CPPFLAGS} -I. -o $@ -c $<
