# Copyright: 2012 iCub Facility, Istituto Italiano di Tecnologia
# Author: Lorenzo Natale
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
# 

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
SET(PROJECTNAME jacub)
PROJECT(${PROJECTNAME})

find_package(YARP)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${YARP_MODULE_PATH})

#add_executable(findLocation findLocation.cpp)
#target_link_libraries(findLocation ${YARP_LIBRARIES})

add_executable(visualPerception visualPerception.cpp)
target_link_libraries(visualPerception ${YARP_LIBRARIES})

add_executable(iDevER iDevER.cpp)
target_link_libraries(iDevER ${YARP_LIBRARIES})

add_executable(propioception PropioceptionModule.cpp)
target_link_libraries(propioception ${YARP_LIBRARIES})

