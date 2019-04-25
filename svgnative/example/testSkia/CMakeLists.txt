# some 3rd party header files used in latest Skia (today is 2019/04/25)
# could not be compiled in C++11.
set(CMAKE_CXX_STANDARD 14)

add_executable(testSkia TestSkia.cpp)

target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/stylesheet/include")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/include/config")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/include/core")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/include/effects")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/include/encode")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/include/gpu")
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../ports/skia")

if(MSVC)
target_link_libraries(testSkia PUBLIC SVGNativeViewerLib "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/lib/skia.lib")
elseif (APPLE)
target_link_libraries(testSkia PUBLIC SVGNativeViewerLib "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/lib/osx/libskia.a")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa")
else()
target_include_directories(testSkia PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/third_party/skcms")
# target_link_libraries(testSkia PUBLIC SVGNativeViewerLib "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/out/Static/libskia.a")
target_link_libraries(testSkia PUBLIC SVGNativeViewerLib "${CMAKE_CURRENT_SOURCE_DIR}/../../../third_party/skia/out/Shared/libskia.so")
endif()
