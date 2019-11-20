lib_LTLIBRARIES = libSVGNativeViewerLib.la

AM_CPPFLAGS = \
	-Wall -pedantic -Wextra \
	-std=c++11 \
	-I$(top_srcdir)/.. \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS)

libSVGNativeViewerLib_la_SOURCES = \
	CSSColorKeywords.h \
	DeprecatedSVGDocumentImpl.cpp \
	SVGDocument.cpp \
	SVGDocumentImpl.cpp \
	SVGDocumentImpl.h \
	SVGParserArcToCurve.cpp \
	SVGStringParser.cpp \
	SVGStringParser.h

libSVGNativeViewerLib_la_LDFLAGS = \
	-version-info @SVGNATIVEVIEWERLIB_VERSION@

