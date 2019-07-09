# Copyright: (C) 2019 Department of Robotics Brain and Cognitive Sciences - Insituto de Matematicas Aplicadas y Sistemas
# Authors: Miguel Estrada
# CopyPolicy: Released under the terms of the GNU GPL v2.0.

set(PROJECTNAME attention)
project(${PROJECTNAME})

find_package(YARP REQUIRED)

# Search for source code.
set(doc_files ${PROJECTNAME}.xml)
#source_group("DOC Files"    FILES ${doc_files})

file(GLOB folder_source src/*.cpp modules/*/src/*.cpp src/*.cpp ../../../src/utils/*.cpp)
file(GLOB folder_header include/*.h)
#source_group("Source Files" FILES ${folder_source})
#source_group("Header Files" FILES ${folder_header})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${YARP_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS}
		    ../../../src/utils/
		    ${PROJECT_SOURCE_DIR}/src
		)
#set (CMAKE_EXE_LINKER_FLAGS -ljsoncpp)
add_executable(${PROJECTNAME} ${folder_header} ${folder_source})
#target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} ${OpenCV_LIBRARIES})
target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} opencv_features2d opencv_flann opencv_imgproc opencv_core -ljsoncpp)
install(TARGETS ${PROJECTNAME} DESTINATION bin)

yarp_install(FILES ${doc_files} DESTINATION "share/iCub/modules")


