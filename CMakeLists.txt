#################################################################################
# FixedPoint Solutions
# 3/8 Bortfield Drive
# 2046 Chiswick, NSW Australia
#
# COPYRIGHT [2013] - [2015], FIXEDPOINT SOLUTIONS
#
# NOTICE: All information contained herein is, and remains the property
# of FixedPoint Solutions and its licensors,if any. The intellectual and
# technical concepts contained herein are proprietary to FixedPoint Solutions
# and its licensors and may be covered by U.S. and Foreign Patents, patents in
# process, and are protected by trade secret or copyright law.
#
# DO NOT DUPLICATE. UNAUTHORIZED DUPLICATION IS A VIOLATION OF STATE, FEDERAL
# AND INTERNATIONAL LAWS. DISSEMINATION OF THIS INFORMATION OR REPRODUCTION OF
# THIS MATERIAL IS PERMITTED ONLY WITH FIXEDPOINT SOLUTIONS PRIOR WRITTEN
# CONSENT.
#
# FIXEDPOINT SOLUTIONS SOFTWARE IS LICENSED OR PROVIDED "AS IS" AND "WITH ALL
# FAULTS,"AND WITHOUT WARRANTY OF ANY KIND, WHETHER EXPRESS, IMPLIED, STATUTORY,
# OR OTHERWISE. FIXEDPOINT SOLUTIONS HEREBY SPECIFICALLY DISCLAIMS ANY IMPLIED
# WARRANTIES OF NON-INFRINGEMENT, TITLE, MERCHANTABILITY, QUIET ENJOYMENT AND
# FITNESS FOR A PARTICULAR PURPOSE, AND ANY WARRANTIES OTHERWISE ARISING OUT OF
# ANY COURSE OF DEALING OR USAGE OF TRADE. FIXEDPOINT SOLUTIONS MAKES NO, AND
# HEREBY EXPRESSLY DISCLAIMS ANY, WARRANTY (A) THAT THE OPERATION OF FIXEDPOINT
# SOLUTIONS SOFTWARE WILL BE CONTINUOUS, UNINTERRUPTED, ERROR-FREE, OR FREE OF
# HARMFUL COMPONENTS, OR (B) THAT FIXEDPOINT SOLUTIONS SOFTWARE WILL MEET
# CUSTOMER'S REQUIREMENTS OR THE REQUIREMENTS OF ANY OF CUSTOMER'S AUTHORIZED
# END USERS.
#################################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 3.0.2)

ENABLE_LANGUAGE(CXX)
CMAKE_POLICY(SET CMP0011 NEW)

PROJECT(AirStream)

###############################################################################
#                                                                             #
# Define global variables                                                     #
#                                                                             #
###############################################################################
IF("${CMAKE_FIND_ROOT_PATH}" STREQUAL "")
  set(CMAKE_FIND_ROOT_PATH "/usr/local")
ENDIF()

set(AIRSTREAM_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
message("Project root folder: " ${AIRSTREAM_PROJECT_ROOT} )

IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Debug)
#   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
message("Build type: " ${CMAKE_BUILD_TYPE} )

# Set custom path variables
set(AIRSTREAM_PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(LIB_AIRSTREAM_PATH ${AIRSTREAM_PROJECT_ROOT}/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -fexceptions -Wall -Wno-unused")
ADD_DEFINITIONS(-DQT_COMPILING_QSTRING_COMPAT_CPP)

###############################################################################
#                                                                             #
# Define sources of libPlayer library                                         #
#                                                                             #
###############################################################################
set(LIB_AIRSTREAM_PATH ${AIRSTREAM_PROJECT_ROOT}/src)
set(LIB_AIRSTREAM_SOURCES
        ${LIB_AIRSTREAM_PATH}/stream.cpp
        ${LIB_AIRSTREAM_PATH}/videoStream.cpp
        ${LIB_AIRSTREAM_PATH}/audioStream.cpp
        )

set(LIB_AIRSTREAM_CAPTURE_PATH ${AIRSTREAM_PROJECT_ROOT}/src/capture)
set(LIB_AIRSTREAM_CAPTURE_SOURCES
#       ${LIB_AIRSTREAM_CAPTURE_PATH}/../../deps/zf_log/zf_log/zf_log.c
        ${LIB_AIRSTREAM_CAPTURE_PATH}/airStream.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/tsSource.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/fileSource.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/dvbSource.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/paketisedPayloadHandler.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/paketisedESPayloadHandler.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/libdvbv5-patch/mpeg_pes.c
        ${LIB_AIRSTREAM_CAPTURE_PATH}/program.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/patHandler.cpp
        ${LIB_AIRSTREAM_CAPTURE_PATH}/pmtHandler.cpp
        )

set(LIB_AIRSTREAM_TRANSCODE_PATH ${AIRSTREAM_PROJECT_ROOT}/src/transcode)
set(LIB_AIRSTREAM_TRANSCODE_SOURCES
#       ${LIB_AIRSTREAM_CAPTURE_PATH}/../../deps/zf_log/zf_log/zf_log.c
        ${LIB_AIRSTREAM_TRANSCODE_PATH}/transcoder.cpp
        ${LIB_AIRSTREAM_TRANSCODE_PATH}/softwareTranscoder.cpp
        )

###############################################################################
#                                                                             #
# Define necessary sources of FDesign                                         #
#                                                                             #
###############################################################################
set(APP_AIRSTREAM_PATH ${AIRSTREAM_PROJECT_ROOT}/src)
set(APP_AIRSTREAM_SOURCES
		${APP_AIRSTREAM_PATH}/main.cpp
		)

###############################################################################
#                                                                             #
# Setup include/link directories                                              #
#                                                                             #
###############################################################################
INCLUDE_DIRECTORIES(
		/usr/include/qt5
		${CMAKE_FIND_ROOT_PATH}/include/qt5
		${AIRSTREAM_PROJECT_ROOT}/lib/QtWidgets.framework/Headers
		${AIRSTREAM_PROJECT_ROOT}/include
		${AIRSTREAM_PROJECT_ROOT}/deps/deps-bass-library/include
		${AIRSTREAM_PROJECT_ROOT}/deps/ilmc-packets-lib/include
		${AIRSTREAM_PROJECT_ROOT}/deps/deps-ctb-library/include
		${AIRSTREAM_PROJECT_ROOT}/deps/libVisualNodes/include
		${AIRSTREAM_PROJECT_ROOT}/deps/poco/include
		${AIRSTREAM_PROJECT_ROOT}/deps/zf_log
		)
LINK_DIRECTORIES(
		${AIRSTREAM_PROJECT_ROOT}/deps/deps-bass-library/lib/${LIB_BASS_PATH}
        ${CMAKE_FIND_ROOT_PATH}/lib
        ${CMAKE_FIND_ROOT_PATH}/local/lib
        /usr/local/lib
		)


###############################################################################
#                                                                             #
# Setup build process                                                         #
#                                                                             #
###############################################################################
ADD_LIBRARY(airstreamCapture STATIC ${LIB_AIRSTREAM_CAPTURE_SOURCES})
ADD_LIBRARY(airstreamTranscode STATIC ${LIB_AIRSTREAM_TRANSCODE_SOURCES})
ADD_LIBRARY(airstream STATIC ${LIB_AIRSTREAM_SOURCES})

ADD_EXECUTABLE(testApp ${APP_AIRSTREAM_SOURCES})

TARGET_LINK_LIBRARIES(testApp airstreamCapture airstreamTranscode airstream avformat avcodec avfilter swscale avresample avutil x264 dvbv5 pthread z bz2)

SET_TARGET_PROPERTIES(airstream
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY ""
)

SET_TARGET_PROPERTIES(airstreamCapture
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY ""
)

SET_TARGET_PROPERTIES(airstreamTranscode
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY ""
)

SET_TARGET_PROPERTIES(testApp
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY ""
)
