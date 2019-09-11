// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Cartesian Interface to control a limb
// in the operational space.
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <yarp/os/Network.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include "modules/attention/src/AttentionModule.cpp"
#include "modules/emotion/src/EmotionModule.cpp"
#include "modules/DevER/src/DevERModule.cpp"
#include "modules/perception/src/PerceptionModule.cpp"
#include <fstream>
#include <iostream>

#include "modules/locomotion/src/Locomotion.cpp"
//#include "modules/memory/src/MemoryModule.cpp"

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

int main(int argc, char *argv[]) {
	Network yarp;

	if (!yarp.checkNetwork()) {
		yError("Yarp server does not seem available\n");
		return 1;
	}

	/*EmotionsModule emotions;
	 AttentionModule attention;
	 LTMemoryModule ltm;
	 WorkingMemoryModule wm;
	 DevERModule dever;
	 PerceptionModule perception;
	 LocomotionModule bodyController;*/

	ResourceFinder rf;
	// rf.setVerbose(); //logs searched directories
	rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
	rf.configure(argc, argv);

	//perception.configure(rf);

	string robotName = rf.check("robot", Value("jacub")).asString();

	//connect module ports
	string from,to;
/*
	 from = "/" + robotName + "/perception/rawImage:i";
	 to = "/icubSim/cam/left";

	if (Network::connect(to, from)) {
		yInfo(
		"Established  port connection from %s to %s",to.c_str(),from.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the iCubSim left cam running?\n",to.c_str(),from.c_str());

	}*/

	from = "/icubSim/skin/left_hand_comp";
		 to = "/"+robotName+"/perception/leftHandSkin:i";

		if (Network::connect(from, to)) {
			yInfo(
			"Established  port connection from %s to %s",from.c_str(),to.c_str());
		}
		else
		{
			yWarning(" Failed establishing connection from %s to %s. Is the iCubSim left cam running?\n",to.c_str(),from.c_str());


		}
		from = "/"+robotName+"/locomotion/leftHandState:o";
				 to = "/"+robotName+"/perception/leftHandState:i";

				if (Network::connect(from, to)) {
					yInfo(
					"Established  port connection from %s to %s",from.c_str(),to.c_str());
				}
				else
				{
					yWarning(" Failed establishing connection from %s to %s. Are the locomotion and perception modules running?\n",to.c_str(),from.c_str());
				}

	 /*to = "/motionCUT/img:i";
		 from = "/icubSim/cam/left";

		if (Network::connect(from, to)) {
			yInfo(
			"Established  port connection from %s to %s",to.c_str(),from.c_str());
		}
		else
		{
			yWarning(" Failed establishing connection from %s to %s. Is the iCubSim left cam and motionCUT modules running?\n",to.c_str(),from.c_str());

		}*/
/*
	from = "/" + robotName + "/perception/rawImage:i";
	to = "/icubSim/cam/left";

	if (Network::connect(to, from)) {
		yInfo(
		"Established  port connection from %s to %s",to.c_str(),from.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the iCubSim left cam running?\n",to.c_str(),from.c_str());

	}
*/
	from = "/" + robotName + "/perception/processedImage:o";
	to = "/yarpview/img:i";

	if (Network::connect(from, to)) {
		yInfo(
		"Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the perception module and yarpview running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/perception/sensorialContext:o";
	to = "/" + robotName + "/attention/sensorialContext:i";

	if (Network::connect(from, to)) {
		yInfo(
		"Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the perception and attention modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/mentalActions:o";
		to = "/" + robotName + "/attention/mentalActions:i";

		if (Network::connect(from, to)) {
			yInfo(
			"Established  port connection from %s to %s",from.c_str(),to.c_str());
		}
		else
		{
			yWarning(" Failed establishing connection from %s to %s. Are the devER and attention modules running?\n",from.c_str(),to.c_str());

		}


	from = "/" + robotName + "/attention/attendedContext:o";
	to = "/" + robotName + "/memory/attendedContext:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the attention and memory modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/memory/attendedContext:o";
	to = "/" + robotName + "/emotion/attendedContext:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Is the attention and memory modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/emotion/emotionalContext:o";
	to = "/" + robotName + "/memory/emotionalContext:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the attention and memory modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/emotion/facialExpression:o";
	to = "/emotion/in";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the emotion interface and %s's emotion module running?\n",from.c_str(),to.c_str(),robotName.c_str());

	}

	from = "/" + robotName + "/memory/emotionalContext:o";
	to = "/" + robotName + "/DevER/currentContext:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/memoryMode:o";
	to = "/" + robotName + "/memory/mode:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/currentContext:o";
	to = "/" + robotName + "/memory/context:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/currentContext:o";
	to = "/" + robotName + "/memory/context:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/matchMode:o";
	to = "/" + robotName + "/memory/matchMode:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	to = "/" + robotName + "/DevER/matchedSchemas:i";
	from = "/" + robotName + "/memory/matchedSchemas:o";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/locomotion/done:o";
	to = "/" + robotName + "/perception/continue:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the locomotion and perception modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/actions:o";
	to = "/" + robotName + "/locomotion/actions:i";

	if (Network::connect(from, to)) {
		yInfo(
		" Established  port connection from %s to %s",from.c_str(),to.c_str());
	}
	else
	{
		yWarning(" Failed establishing connection from %s to %s. Are the locomotion and DevER modules running?\n",from.c_str(),to.c_str());

	}

	from = "/" + robotName + "/DevER/expectations:o";
		//to = "/" + robotName + "/memory/expectations:i";
	to = "/" + robotName + "/emotion/expectations:i";
		if (Network::connect(from, to)) {
			yInfo(
			" Established  port connection from %s to %s",from.c_str(),to.c_str());
		}
		else
		{
			yWarning(" Failed establishing connection from %s to %s. Are the memory and DevER modules running?\n",from.c_str(),to.c_str());

		}

		/*from = "/" + robotName + "/memory/expectations:o";
			to = "/" + robotName + "/emotion/expectations:i";

			if (Network::connect(from, to)) {
				yInfo(
				" Established  port connection from %s to %s",from.c_str(),to.c_str());
			}
			else
			{
				yWarning(" Failed establishing connection from %s to %s. Are the memory and emotions modules running?\n",from.c_str(),to.c_str());

			}*/

			from = "/" + robotName + "/emotion/emotionalContext:o";
				to = "/" + robotName + "/attention/emotionalContext:i";

				if (Network::connect(from, to)) {
					yInfo(
					" Established  port connection from %s to %s",from.c_str(),to.c_str());
				}
				else
				{
					yWarning(" Failed establishing connection from %s to %s. Are the emotion and attention modules running?\n",from.c_str(),to.c_str());

				}

				from = "/" + robotName + "/perception/blobbedImage:o";
					to = "/motionCUT/img:i";

					if (Network::connect(from, to)) {
						yInfo(
						" Established  port connection from %s to %s",from.c_str(),to.c_str());
					}
					else
					{
						yWarning(" Failed establishing connection from %s to %s. Are the perception and motionCUT modules running?\n",from.c_str(),to.c_str());

					}

					from = "/motionCUT/blobs:o";
						to = "/" + robotName + "/perception/movingBlobs:i";

						if (Network::connect(from, to)) {
							yInfo(
							" Established  port connection from %s to %s",from.c_str(),to.c_str());
						}
						else
						{
							yWarning(" Failed establishing connection from %s to %s. Are the perception and motionCUT modules running?\n",from.c_str(),to.c_str());

						}


}

