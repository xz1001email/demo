CMAKE_MINIMUM_REQUIRED (VERSION 2.6)

set(CMAKE_BUILD_TYPE "Debug")

SET(CMAKE_CXX_FLAGS_RELEASE "-Wno-unused-result -Wno-narrowing -DCR_DEBUG_FLAG -O3 -Wall -std=gnu++0x -std=c++11")
#SET(CMAKE_CXX_FLAGS_DEBUG "-Wno-unused-result -Wno-narrowing -Wno-unused-variable -Wall -DCR_DEBUG_FLAG -g -std=gnu++0x -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-Wno-unused-result -Wno-narrowing -Wno-unused-variable -Wall -DCR_DEBUG_FLAG -g")

IF (USING_ALL)
    # 查找当前目录下的所有源文件，并将名称保存到 DIR_SRCS 变量
    AUX_SOURCE_DIRECTORY(. DIR_SRCS)
    # 指定生成目标
    ADD_EXECUTABLE(cond_test ${DIR_SRCS})
    TARGET_LINK_LIBRARIES(cond_test pthread)
ELSE()
    ADD_EXECUTABLE(cond_test message.c main.c)
    TARGET_LINK_LIBRARIES(cond_test pthread)

    ADD_EXECUTABLE(timedwait timedwait.c)
    TARGET_LINK_LIBRARIES(timedwait pthread)
ENDIF()




