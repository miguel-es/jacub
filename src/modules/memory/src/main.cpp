/*
 * main.cpp
 *
 *  Created on: Jun 26, 2019
 *      Author: miguel
 */

#include <MemoryModule.cpp>
int main(int argc, char *argv[]) {
	Network yarp;

	if (!yarp.checkNetwork()) {
		yError("Yarp server does not seem available\n");
		return 1;
	}

	MemoryModule memory;

	ResourceFinder rf;
	rf.setVerbose(); //logs searched directories
	rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
	rf.configure(argc, argv);

	memory.runModule(rf);

	//WorkingMemoryModule wMemory;

	//wMemory.runModule(rf);
}
