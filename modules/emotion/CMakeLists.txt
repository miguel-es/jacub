# Copyright: (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
# Authors: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
set(PROJECTNAME emotion)
project(${PROJECTNAME})

find_package(YARP REQUIRED)

#find_package(JSONCPP jsoncpp)
#link_libraries(${JSONCPP_LIBRARIES})

# Search for source code.
set(doc_files ${PROJECTNAME}.xml)
source_group("DOC Files"    FILES ${doc_files})

file(GLOB folder_source src/*.cpp src/*.cc src/*.c)
file(GLOB folder_header include/iCub/*.h)
source_group("Source Files" FILES ${folder_source})
source_group("Header Files" FILES ${folder_header})

include_directories(${PROJECT_SOURCE_DIR}/include
                    ${YARP_INCLUDE_DIRS}
                    ${OpenCV_INCLUDE_DIRS}
		    ../../src/utils/)
	    #set (CMAKE_EXE_LINKER_FLAGS -ljsoncpp)
add_executable(${PROJECTNAME} ${folder_header} ${folder_source})
#link_libraries( -ljsoncpp)
target_link_libraries(${PROJECTNAME} ${YARP_LIBRARIES} ${OpenCV_LIBRARIES} ${JSONCPP_LIBRARIES} -ljsoncpp)

install(TARGETS ${PROJECTNAME} DESTINATION bin)

yarp_install(FILES ${doc_files} DESTINATION "share/iCub/modules")


