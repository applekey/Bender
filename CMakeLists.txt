# CMake entry point
cmake_minimum_required (VERSION 2.6)
project (BENDER)

find_package(OpenGL REQUIRED)

# Compile external dependencies 
add_subdirectory (external)

# On Visual 2005 and above, this module can set the debug working directory
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/external/rpavlik-cmake-modules-1c73e35")
include(CreateLaunchers)
include(MSVCMultipleProcessCompile) # /MP

if(INCLUDE_DISTRIB)
	add_subdirectory(distrib)
endif(INCLUDE_DISTRIB)

## FIND BOOST
set(Boost_USE_STATIC_LIBS        ON)
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/external/boost_1_54_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/external/boost_1_54_0/libs")
#set(Boost_DEBUG TRUE)
FIND_PACKAGE(Boost 1.54 COMPONENTS thread system chrono date_time atomic )


## FIND POINT GREY
SET( POINTGREY_NAMES FlyCapture2 )
FIND_PATH( POINTGREY_INCLUDE_DIR FlyCapture2.h
            PATHS "C:/Program\ Files (x86)/Point\ Grey\ Research/FlyCapture2/include" 
            ${COMPILER_PATH}/PlatformSDK/Include )

        FIND_LIBRARY( 	POINTGREY_LIBRARIES
            NAMES ${POINTGREY_NAMES}
            PATHS "C:/Program\ Files (x86)/Point\ Grey\ Research/FlyCapture2/lib" 
            ${COMPILER_PATH}/PlatformSDK/Lib )      

			
## OPENCV
FIND_LIBRARY( OPENCV_LIBS
		NAMES opencv_core245d 
	    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/build/x86/vc10/lib
	 )   
	  
FIND_LIBRARY( OPENCV_LIBS2
NAMES  opencv_highgui245d
HINTS ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/build/x86/vc10/lib
)    

include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/external/glfw-2.7.6/include/
	${CMAKE_CURRENT_SOURCE_DIR}/external/glm-0.9.4.0/
	${CMAKE_CURRENT_SOURCE_DIR}/external/glew-1.9.0/include/
	${FLYCAPTURE_INCLUDE_PATH}
	${POINTGREY_INCLUDE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/common
	${Boost_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/build/include
	${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/build/include/opencv
	${CMAKE_CURRENT_SOURCE_DIR}/external/opencv/build/include/opencv2
	
	
	${DIR}/headers /opt/local/include
	
)

link_directories (${DIR}/src/utils /opt/local/lib)

get_property(inc_dirs DIRECTORY PROPERTY INCLUDE_DIRECTORIES)

set(ALL_LIBS
	${OPENGL_LIBRARY}
	GLFW_276
	GLEW_190
	${POINTGREY_LIBRARIES}
	${Boost_LIBRARIES}
	${OPENCV_LIBS}
	${OPENCV_LIBS2}
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_core245d.lib
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_calib3d245d.lib
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_imgproc245d.lib
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_highgui245d.lib
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_features2d245d.lib
	C:/Users/vincent/Documents/Ece496/Code/Bender_Opengl/external/opencv/build/x86/vc10/lib/opencv_flann245d.lib
	"C:/Program\ Files\ (x86)/Point\ Grey\ Research/FlyCapture2/lib/FlyCapture2d_v100.lib"
)

# message("opencvlibs = ${ANTTWEAKBAR_LIBS}")



add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
	-DHAS_BOOST
)

add_executable(Bender 
	Bender/main.cpp Bender/pgcalib.cpp Bender/pgcalib.h Bender/common.h Bender/queue.cpp Bender/queue.h 
	Bender/depth.frag Bender/passThrough.vert
	Bender/wrap.frag Bender/wrap.vert
	Bender/gaussPass1.vert Bender/gauss.frag
	Bender/gaussPass2.vert Bender/gauss2.frag
	Bender/normal.frag Bender/normal.vert
	Bender/display.frag Bender/display.vert
	Bender/mask.frag
	
	Bender/median.frag
	Bender/median.vert
	Bender/mesh.frag
	Bender/mesh.vert
)
target_link_libraries(Bender
	${ALL_LIBS}
)
# Xcode and Visual working directories
create_target_launcher(Bender WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/Bender/")

SOURCE_GROUP(common REGULAR_EXPRESSION ".*/common/.*" )

add_custom_command(
   TARGET Bender POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/Bender${CMAKE_EXECUTABLE_SUFFIX}" "${CMAKE_CURRENT_SOURCE_DIR}/Bender/"
)



