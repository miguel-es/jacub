// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 * @ingroup icub_tutorials
 * \defgroup imageProc imageProc
 */

#include <stdio.h>
#include <iostream>
#include <yarp/os/Network.h>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <yarp/os/impl/Logger.h>
#include <jutils.h>

#define SWITCH_TO_V "changeAttentionToV"
#define SWITCH_TO_T "changeAttentionToT"
using namespace std;
using namespace yarp::os;

class AttentionModule: public RFModule {
private:
	string robotName;
	Port sensorialContextInputPort;
	BufferedPort<Bottle> attentionSwitchInputPort;
	Port attendedContextOutputPort;
	Json::Reader jsonReader;
	Json::FastWriter jsonWriter;
	Json::Value attendedContext;


public:
	AttentionModule(){
		jsonReader.parse("[]", attendedContext);
	}

	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		printf("robot name: %s\n", robotName.c_str());

		if (!sensorialContextInputPort.open(
				"/" + robotName + "/attention/sensorialContext:i")) {
			yError("Failed creating input port for sensorial context");
			return false;
		}

		if (!attentionSwitchInputPort.open(
				"/" + robotName + "/attention/attentionSwitch:i")) {

			yError("Failed creating input port for attention switch");
			return false;
		}

		if (!attendedContextOutputPort.open(
				"/" + robotName + "/attention/attendedContext:o")) {
			yError("Failed creating output port for attended context");
			return false;
		}

		return true;
	}

	virtual bool close() {
		 sensorialContextInputPort.close();
		 attentionSwitchInputPort.close();
		 attendedContextOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		Bottle input;
				Bottle *buffInput;
				string input_string;
				std::cout << "getting attention switch...\n";
				//attentionSwitchInputPort.read(false)
				buffInput = attentionSwitchInputPort.read(false);// non blocking read
				if(buffInput!=NULL){
					std::cout << "Got: " << buffInput->toString() << '\n';
				}else{
					std::cout << "attention switch not set \n";
				}


				std::cout << "waiting for sensorial context...\n";
input.clear();
								sensorialContextInputPort.read(input);
								Json::Value sensorialContext;
								yDebug("read %s\n",input.toString().c_str());
								input_string = input.toString();
								prepareInput(input_string);
								yDebug("read cleaned%s\n",input.toString().c_str());
								jsonReader.parse(input_string.c_str(), sensorialContext);
if(sensorialContext[0].size()!=0){


								std::cout << "Got sensorial context: " << '\n' << sensorialContext.toStyledString()
										<< '\n';
								Json::Value attendedVisual;
								for (Json::Value obj : sensorialContext[0]) {
									if(obj["color"]=="c1"){
										attendedVisual = obj;
									}
								}
								attendedContext[0] = attendedVisual;
}else{
	attendedContext[0] = sensorialContext[0];}

							attendedContext[1] = sensorialContext[1];
//}else{attendedContext = sensorialContext;}
							std::cout << "Attended context: " << '\n' << attendedContext.toStyledString()
									<< '\n';

							Bottle output;
							output.addString(jsonWriter.write(attendedContext));
							yDebug(" Writing out attended context");
							attendedContextOutputPort.write(output);
		return true;
	}
};

