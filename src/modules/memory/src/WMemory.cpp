// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <yarp/os/all.h>
#include <jsoncpp/json/json.h>
#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <list>
#include <iostream>
#include <jutils.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class WMemoryThread: public RateThread {
	Port attendedContextInputPort;
	Port attendedContextOutputPort;
	Port emotionalContextInputPort;
	Port emotionalContextOutputPort;
	Port expectationsInputPort;
	Port expectationsOutputPort;

	Json::Reader jsonReader;
	Json::FastWriter fastWriter;
	Json::Value context;
	Json::Value expectations;
	Json::Value actions;
	list<Json::Value> stm;

	string robotName;
	int cycles;
public:
	WMemoryThread(string robotName, const double period) :
			RateThread(int(period * 1000.0)) {
		this->robotName = robotName;
		jsonReader.parse("{\"schemes\":[]}", context);
		jsonReader.parse("[{},{}]", expectations);
		cycles = 0;

	}

	virtual bool threadInit() {
		if (!attendedContextInputPort.open(
				"/" + robotName + "/memory/attendedContext:i")) {
			printf("Failed creating input port for attended context");
			return false;
		}

		if (!attendedContextOutputPort.open(
				"/" + robotName + "/memory/attendedContext:o")) {
			printf("Failed creating output port for attended context");
			return false;
		}

		if (!emotionalContextInputPort.open(
				"/" + robotName + "/memory/emotionalContext:i")) {
			printf("Failed creating input port for emotional context");
			return false;
		}

		if (!emotionalContextOutputPort.open(
				"/" + robotName + "/memory/emotionalContext:o")) {
			printf("Failed creating output port for emotional context");
			return false;
		}

		if (!expectationsInputPort.open(
						"/" + robotName + "/memory/expectations:i")) {
					printf("Failed creating input port for expectations");
					return false;

		}

		if (!expectationsOutputPort.open(
						"/" + robotName + "/memory/expectations:o")) {
					printf("Failed creating output port for expectations");
					return false;


		}

		return true;

	}
	virtual void afterStart(bool s) {
		if (s)
			printf("Working Memory thread started successfully\n");
		else
			printf("Working Memory thread did not start\n");
	}

	virtual void run() {
		yDebug(" WMemory: cycle %d",++cycles);

		Bottle input;
		Bottle output;
		string input_string="[{},{}]";

		yDebug(" WMemory: waiting for attended context...\n");
		attendedContextInputPort.read(input);
		input_string = input.toString();
		//yDebug(" WMemory: Got input: %s",input_string.c_str());

		prepareInput(input_string);
		jsonReader.parse(input_string.c_str(), context);
		yDebug(" WMemory: Got context: %s",context.toStyledString().c_str());

		yDebug(" WMemory: writing out attended context...\n");
		output.addString(fastWriter.write(context));
		attendedContextOutputPort.write(output);

		yDebug(" WMemory: waiting for emotional context...\n");
		input.clear();
		emotionalContextInputPort.read(input);
		input_string = input.toString();
		prepareInput(input_string);
		jsonReader.parse(input_string.c_str(), context);
		yDebug(" WMemory: Got context: %s",context.toStyledString().c_str());
		output.clear();
		yDebug(" WMemory: writing out emotional context...\n");
				output.addString(fastWriter.write(context));
				emotionalContextOutputPort.write(output);

				yDebug(" WMemory: waiting for expectations...\n");
				input.clear();
				//expectationsInputPort.read(input);
				//yDebug(" WMemory: input read\n");
				//input_string = input.toString();
				//prepareInput(input_string);
				//jsonReader.parse(input_string.c_str(), expectations);
				yDebug(" WMemory: Got expectations: %s",expectations.toStyledString().c_str());
				yDebug(" WMemory: writing out expectations...\n");
				output.addString(fastWriter.write(expectations));
				expectationsOutputPort.write(output);
				Json::Value timeStep;
				jsonReader.parse("[]", timeStep);
				timeStep.append(context);
				timeStep.append(actions);
				timeStep.append(expectations);
				stm.push_back(timeStep);
				/*output.clear();
				printf("writing out emotional context...\n");
						output.addString(fastWriter.write(context));
						emotionalContextOutputPort.write(output);*/

	}

	virtual void threadRelease() {
		attendedContextInputPort.close();
		attendedContextOutputPort.close();
		emotionalContextInputPort.close();
		emotionalContextOutputPort.close();
		expectationsInputPort.close();
	}
};

