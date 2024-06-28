cmake_minimum_required(VERSION 3.27.9)

#project variables
set(PROJECT_NAME threadx-cpp-test-app)
set(VERSION_MAJOR 0) #when you make incompatible API or HW changes
set(VERSION_MINOR 1) #when you add functionality in a backwards compatible manner
set(VERSION_PATCH 0) #post-release version. when you make backwards compatible bug fixes. must be 0 for release and release candidate versions

set(MCU_FAMILY STM32L4xx) #MCU HAL driver folder
set(MCU_LINE STM32L496xx) #MCU line define macro
set(MCU_TYPE STM32L496ZGTX) #Linker/startup filenames

set(THREADX_VER v6.4.1_rel)

find_package(Git REQUIRED)
Include(FetchContent)

set(GITHUB_SUBDIR https://github.com/HosseinSagha)
FetchContent_Declare(toolchain-gnu
                     GIT_REPOSITORY ${GITHUB_SUBDIR}/toolchain-gnu.git
                     GIT_TAG origin/main
                     SYSTEM)
FetchContent_MakeAvailable(toolchain-gnu)
FetchContent_GetProperties(toolchain-gnu SOURCE_DIR TOOLCHAIN_DIR)

#set toolchain and project
set(CMAKE_TOOLCHAIN_FILE ${TOOLCHAIN_DIR}/cortex-m4f.cmake)

project(${PROJECT_NAME}
        LANGUAGES C CXX ASM
        VERSION ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH})

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release) #default build type is release
endif()

add_compile_definitions(${MCU_LINE}
                        SYSTEM_CLOCK=4000000
                        ) 

#link libs and dirs
set(LINK_DIRS ${PROJECT_SOURCE_DIR}/app/Middlewares/ST/ARM/DSP/Lib)

configure_file(${PROJECT_SOURCE_DIR}/app/tx_user.h.in ${FETCHCONTENT_BASE_DIR}/tx_user.h)
configure_file(${PROJECT_SOURCE_DIR}/app/fx_user.h.in ${FETCHCONTENT_BASE_DIR}/fx_user.h)
configure_file(${PROJECT_SOURCE_DIR}/app/lx_user.h.in ${FETCHCONTENT_BASE_DIR}/lx_user.h)

set(LIBS
    stm32_hal_drivers
    arm_cortexM4lf_math
    threadx-cpp
    )
    
#include directories
set(SYS_INC_DIRS
    ${PROJECT_SOURCE_DIR}/app/Middlewares/ST/ARM/DSP/Inc
    )

set(INC_DIRS
    ${PROJECT_SOURCE_DIR}/app
    )

#find all source files
file(GLOB USER_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/app/*.cpp)
file(GLOB CORE_SRCS CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/app/Core/Src/*.c)
set(CORE_SRCS ${CORE_SRCS} ${PROJECT_SOURCE_DIR}/app/Core/Src/tx_initialize_low_level.S)
set_source_files_properties(${CORE_SRCS} PROPERTIES COMPILE_FLAGS -w) #disable warnings for system sources

#object library
add_library(SRC_OBJS OBJECT ${USER_SRCS} ${CORE_SRCS})
target_include_directories(SRC_OBJS SYSTEM PUBLIC ${SYS_INC_DIRS})
target_include_directories(SRC_OBJS PUBLIC ${INC_DIRS} ${PROJECT_BINARY_DIR})
target_link_libraries(SRC_OBJS PRIVATE ${LIBS}) #add dependencies so it sees libs' public properties

set(LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/app/${MCU_TYPE}_FLASH) #linker file

#startup source
string(TOLOWER ${MCU_TYPE} MCU_TYPE)
set(STARTUP_SOURCE ${PROJECT_SOURCE_DIR}/app/Core/Startup/startup_${MCU_TYPE})

#add libs
add_subdirectory(${PROJECT_SOURCE_DIR}/app/Drivers)
add_subdirectory(${PROJECT_SOURCE_DIR}/lib)

#read version number
if(CMAKE_BUILD_TYPE STREQUAL Release AND DEFINED ENV{CI_PIPELINE_ID})
    set(VERSION_SUFFIX -${PROJECT_VERSION})
endif()

configure_file(${PROJECT_SOURCE_DIR}/app/version.hpp.in version.hpp)
set(EXEC_NAME ${PROJECT_NAME}${VERSION_SUFFIX})

#add project binaries
set(EXECUTABLE ${EXEC_NAME}.elf)
add_executable(${EXECUTABLE} ${STARTUP_SOURCE}.s)

#link libraries
target_link_directories(${EXECUTABLE} PRIVATE ${LINK_DIRS})
target_link_libraries(${EXECUTABLE} PRIVATE ${LIBS} SRC_OBJS)

#target specific link options
target_link_options(${EXECUTABLE} PRIVATE
                    ${LINKER_OPTS}
                    LINKER:-Map=${CMAKE_BINARY_DIR}/${EXEC_NAME}.map
                    -T${LINKER_SCRIPT}${EXEC_SUFFIX}.ld)

#print executable size and create hex and bin files
add_custom_command(TARGET ${EXECUTABLE}
                    POST_BUILD
                    COMMAND ${CMAKE_SIZE_UTIL} ${EXECUTABLE}
                    COMMAND ${CMAKE_OBJCOPY} -O ihex ${EXECUTABLE} ${EXEC_NAME}.hex
                    COMMAND ${CMAKE_OBJCOPY} -O binary ${EXECUTABLE} ${EXEC_NAME}.bin
                    VERBATIM)
