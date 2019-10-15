add_executable(testCairo TestCairo.cpp)

target_link_libraries(testCairo PUBLIC SVGNativeViewerLib)
target_link_libraries(testCairo PUBLIC "${CAIRO_LIBRARIES}")
target_link_libraries(testCairo PUBLIC "-lpng16")
target_link_libraries(testCairo PUBLIC "-lpthread")
target_link_libraries(testCairo PUBLIC "-lpixman-1")
target_link_libraries(testCairo PUBLIC "-lz")
