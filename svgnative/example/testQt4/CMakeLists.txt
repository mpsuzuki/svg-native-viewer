add_executable(testQt4 TestQt4.cpp)

target_link_libraries(testQt4 PUBLIC SVGNativeViewerLib)
target_link_libraries(testQt4 PUBLIC "${QT4_LIBRARIES}")
