add_executable(testQt TestQt.cpp)

target_link_libraries(testQt PUBLIC SVGNativeViewerLib)
target_link_libraries(testQt PUBLIC "${QT_LIBRARIES}")
