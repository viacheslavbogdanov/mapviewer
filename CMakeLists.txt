cmake_minimum_required(VERSION 3.8)

project(MapViewer)

include_directories(MapViewer/sdk/libpng/include
                    MapViewer/sdk/og
                    MapViewer/sdk/OpenGL2/include
                    MapViewer/sdk/earcut/include
                    MapViewer/sdk/vtzero/include-external
                    MapViewer/sdk/vtzero/include
                    MapViewer/sdk/protozero/include
)

add_executable(MapViewer WIN32
    MapViewer/ElevationMap.cpp
    MapViewer/ElevationMap.h
    MapViewer/GLRenderer.cpp
    MapViewer/GLRenderer.h
    MapViewer/MeshConstructor.cpp
    MapViewer/MeshConstructor.h
    MapViewer/MeshSubdivision.cpp
    MapViewer/MeshSubdivision.h
    MapViewer/Tesselator.cpp
    MapViewer/Tesselator.h
    MapViewer/Utils.cpp
    MapViewer/Utils.h
    MapViewer/Viewer.cpp
    MapViewer/VTZeroRead.cpp
    MapViewer/VTZeroRead.h
    MapViewer/Scene.cpp
    MapViewer/Scene.h
    # helper library
    MapViewer/sdk/og/IOGAabb.h
    MapViewer/sdk/og/IOGCamera.h
    MapViewer/sdk/og/IOGCoreHelpers.h
    MapViewer/sdk/og/IOGFrustum.h
    MapViewer/sdk/og/IOGMath.h
    MapViewer/sdk/og/IOGMatrix.h
    MapViewer/sdk/og/IOGObb.h
    MapViewer/sdk/og/IOGPlane.h
    MapViewer/sdk/og/IOGQuaternion.h
    MapViewer/sdk/og/IOGVector.h
    MapViewer/sdk/og/IOGVertexBuffers.h
    MapViewer/sdk/og/ogcamera.cpp
    MapViewer/sdk/og/ogcamera.h
    MapViewer/sdk/og/ogmatrix.cpp
    MapViewer/sdk/og/ogquaternion.cpp
    MapViewer/sdk/og/ogshader.cpp
    MapViewer/sdk/og/ogshader.h
    MapViewer/sdk/og/ogvector.cpp
    MapViewer/sdk/og/ogvertexbuffers.cpp
    MapViewer/sdk/og/ogvertexbuffers.h
    MapViewer/sdk/og/OpenGL2.h
)

target_compile_definitions(MapViewer PRIVATE
                -D_CRT_SECURE_NO_WARNINGS
                -D_UNICODE
                -DNOMINMAX
                -DGLEW_STATIC
)

if(${CMAKE_SIZEOF_VOID_P} STREQUAL "8")
    set(PNGLIBPATH MapViewer/sdk/libpng/lib/x64)
    set(GLEWPATH MapViewer/sdk/OpenGL2/lib/x64)
else()
    set(PNGLIBPATH MapViewer/sdk/libpng/lib/x86)
    set(GLEWPATH MapViewer/sdk/OpenGL2/lib/x86)
endif()

target_link_directories(MapViewer PRIVATE 
                        ${PNGLIBPATH}
                        ${GLEWPATH}
)

target_link_libraries(MapViewer libpng16_static zlibstatic glew32s opengl32)

set(MapViewerDir $<TARGET_FILE_DIR:MapViewer>)

add_custom_command(TARGET MapViewer POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/assets ${MapViewerDir}/assets/
)
