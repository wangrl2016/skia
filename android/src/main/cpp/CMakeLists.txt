# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include/entt/)

# ffmpeg libraries header
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include/ffmpeg/)

set(FFMPEG_LIBS avformat avcodec avdevice avfilter avutil postproc swresample swscale)

foreach (LIB ${FFMPEG_LIBS})
    message(${LIB})
    add_library(${LIB}
            SHARED
            IMPORTED)
    set_target_properties( # Specifies the target library.
            ${LIB}

            # Specifies the parameter you want to define.
            PROPERTIES
            IMPORTED_LOCATION

            # Provides the path to the library you want to import.
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/arm64-v8a/lib${LIB}.so)

endforeach ()

# third party libraries
set(THIRD_SO_LIBS mp3lame)

foreach (LIB ${THIRD_SO_LIBS})
    message(${LIB})
    add_library(${LIB}
            SHARED
            IMPORTED)
    set_target_properties( # Specifies the target library.
            ${LIB}

            # Specifies the parameter you want to define.
            PROPERTIES
            IMPORTED_LOCATION

            # Provides the path to the library you want to import.
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/arm64-v8a/lib${LIB}.so)

endforeach ()

set(THIRD_STATIC_LIBS x264)

foreach (LIB ${THIRD_STATIC_LIBS})
    message(${LIB})
    add_library(${LIB}
            STATIC
            IMPORTED)
    set_target_properties( # Specifies the target library.
            ${LIB}

            # Specifies the parameter you want to define.
            PROPERTIES
            IMPORTED_LOCATION

            # Provides the path to the library you want to import.
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/arm64-v8a/lib${LIB}.a)

endforeach ()

# skia libraries header
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include/skia/)

set(SKIA_STATIC_LIBS
        cpu-features
        dng_sdk
        expat
        freetype2
        harfbuzz
        icu
        jpeg
        piex
        png
        sfntly
        skia
        skottie
        skresources
        sksg
        skshaper
        webp
        webp_sse41
        zlib
        zlib_x86)

foreach(LIB ${SKIA_STATIC_LIBS})
    message(${LIB})

    add_library(${LIB}
            STATIC
            IMPORTED)

    set_target_properties( # Specifies the target library.
            ${LIB}

            # Specifies the parameter you want to define.
            PROPERTIES
            IMPORTED_LOCATION

            # Provides the path to the library you want to import.
            ${CMAKE_CURRENT_SOURCE_DIR}/../../../libs/arm64-v8a/lib${LIB}.a)
endforeach()

# bfx libraries header
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../../include/bfx/)

add_library( # Sets the name of the library.
        native-lib

        # Sets the library as a shared library.
        SHARED
        # Provides a relative path to your source file(s).
        NativeRendererInterface.cpp

        bfx/Engine.cpp

        bfx/Systems/ImageOutputSystem.cpp
        bfx/Systems/RenderSystem.cpp
        bfx/Systems/ResourceSystem.cpp
        bfx/Systems/VideoGenerateSystem.cpp
        bfx/Utils/AudioFifo.cpp
        )

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
        log-lib

        # Specifies the name of the NDK library that
        # you want CMake to locate.
        log)

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
        native-lib

        ${FFMPEG_LIBS}
        ${SKIA_STATIC_LIBS}
        # Links the target library to the log library
        # included in the NDK.
        ${log-lib})
