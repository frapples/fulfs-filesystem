PROJECT (UNIXLIKEFS)

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "$ENV{CFLAGS} -O0 -Wall -g -ggdb -std=c99")
set(CMAKE_C_FLAGS_RELEASE "$ENV{CFLAGS} -O3 -Wall -std=c99")



SET(LIB_LIST memory/alloc.c utils/sys.c utils/testtools.c utils/log.c utils/path.c datastruct/string.c
  shell.c shell_command.c
  fs.c
  device_io.c)

aux_source_directory("fulfs/" FULFS_SRCS)



MESSAGE(STATUS "This is BINARY dir" ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "This is SOURCE dir" ${PROJECT_SOURCE_DIR})


# 相关的源文件是SRC_LIST中定义的源文件列表
ADD_EXECUTABLE(main main.c ${LIB_LIST} ${FULFS_SRCS})


# 测试
ENABLE_TESTING()
ADD_EXECUTABLE(my_test test.c ${LIB_LIST} ${FULFS_SRCS})
ADD_TEST(my_test my_test)
