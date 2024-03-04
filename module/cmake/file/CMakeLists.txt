CMAKE_MINIMUM_REQUIRED(VERSION 2.8.2)
PROJECT(common_test)

#SET(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-result -Wno-narrowing -DCR_DEBUG_FLAG -O3 -Wall -std=gnu++0x")
#SET(CMAKE_CXX_FLAGS_DEBUG "-Wno-unused-result -Wno-narrowing -Wno-unused-variable -Wall -DCR_DEBUG_FLAG -g -std=gnu++0x")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-result -Wno-narrowing -DCR_DEBUG_FLAG -O3 -Wall -std=gnu++0x -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wno-unused-result -Wno-narrowing -Wno-unused-variable -Wall -DCR_DEBUG_FLAG -g -std=gnu++0x -std=c++11")

SET(PROJECT_ROOT_PATH "${CMAKE_SOURCE_DIR}")
MESSAGE(STATUS "${CMAKE_SOURCE_DIR}")
SET(EXECUTABLE_OUTPUT_PATH "${PROJECT_BINARY_DIR}/bin")
SET(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib/")
SET(SCRIPTS_PATH "${PROJECT_ROOT_PATH}/../scripts/")
#SET(CMAKE_SHARED_LINKER_FLAGS "-Wl,--export-all-symbols")
#SET(CMAKE_STATIC_LINKER_FLAGS "-Wl,--export-all-symbols")
SET(LINK_FLAGS ${LINK_FLAGS} "-Wl,-whole-archive")

MESSAGE(STATUS ${PROJECT_ROOT_PATH})

#root
INCLUDE_DIRECTORIES("${PROJECT_ROOT_PATH}")
IF ( "${CMAKE_SIZEOF_VOID_P}" EQUAL "4")
    MESSAGE(STATUS "x32 platform")
    SET(PLATFORM x86)
ELSEIF( "${CMAKE_SIZEOF_VOID_P}" EQUAL "8")
    MESSAGE(STATUS "x64 platform")
    SET(PLATFORM x64)
ENDIF()

# opencv package required

IF (MSVC)
ELSEIF (ANDROID_ABI)
    MESSAGE(STATUS "ANDORID platform")
    ADD_DEFINITIONS(-DANDROID)
    ADD_DEFINITIONS(-DUSE_GLOG)
    ADD_DEFINITIONS(-DUSE_CAMERA)
    ADD_DEFINITIONS(-DANDROID_CAM_RECORD)
    SET(USE_CAN 1)
    ADD_CUSTOM_TARGET(MINIEYE_VERSION COMMAND ${SCRIPTS_PATH}/version.sh |tee ${PROJECT_ROOT_PATH}/version.h.in)
ELSEIF (UNIX)
ENDIF()


#ADD_SUBDIRECTORY(common/base)
#ADD_SUBDIRECTORY(common/concurrency)
#ADD_SUBDIRECTORY(common/system)
#ADD_SUBDIRECTORY(common/time)
#ADD_SUBDIRECTORY(common/file_operate)
#ADD_SUBDIRECTORY(common/memory)
#ADD_SUBDIRECTORY(common/string)
#ADD_SUBDIRECTORY(common/hal)
#ADD_SUBDIRECTORY(common/ringbuf)
#ADD_SUBDIRECTORY(common/mp4)
#ADD_SUBDIRECTORY(common/mjpeg)
IF (ANDROID_ABI)
    #ADD_SUBDIRECTORY(thirdparty/miniglog)
ENDIF()


SET(MAIN "scanDir")

#link library
IF (WIN32)
ELSEIF(ANDROID_ABI)

    #SET(SRC_LIST RingBufDebug.cpp CRingBuf.cpp)
    #ADD_LIBRARY(ringbuf STATIC ${SRC_LIST})

    ADD_EXECUTABLE(${MAIN} display_dir.cpp)
    #ADD_DEPENDENCIES(wsi MINIEYE_VERSION)
    TARGET_LINK_LIBRARIES(${MAIN}
        #        halio concurrency
        -Wl,--start-group
        -Wl,--end-group
        )
ELSE()
    ADD_EXECUTABLE(audioplayer ./audioplayer_linux.cpp)
    TARGET_LINK_LIBRARIES(audioplayer
        halio
        -Wl,--start-group
        -Wl,--end-group
        )
endif()


