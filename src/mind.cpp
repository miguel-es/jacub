// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Cartesian Interface to control a limb
// in the operational space.
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <yarp/os/Network.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <iostream>

#include "modules/emotion/src/EmotionModule.cpp"

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;


int main(int argc, char *argv[])
{
    Network yarp;

    if (!yarp.checkNetwork())
    {
        yError("Yarp server does not seem available\n");
        return 1;
    }

    Emotions emotions;
    Memory memory;
    iDevER idever;
    Perception perception;
    bodyController bodyController;

    ResourceFinder rf;
    rf.setVerbose(); //logs searched directories
    rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
    rf.configure(argc,argv);

    memory.runModule(rf);
    // return 0;
}

