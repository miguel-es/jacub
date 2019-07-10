/*
 * main.cpp
 *
 *  Created on: Jun 26, 2019
 *      Author: miguel
 */
#include <Locomotion.cpp>

int main(int argc, char *argv[])
{
    Network yarp;

    if (!yarp.checkNetwork())
    {
        yError(" yarp server does not seem available\n");
        return 1;
    }

    LocomotionModule locomotion;

    ResourceFinder rf;
    rf.setVerbose();
    rf.setDefaultConfigFile("config.ini");
    rf.configure(argc,argv);



     locomotion.runModule(rf);
}
