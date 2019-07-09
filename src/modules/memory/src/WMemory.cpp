// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <yarp/os/all.h>
#include <jsoncpp/json/json.h>
#include <yarp/os/impl/Logger.h>

#include <fstream>
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

	Json::Reader jsonReader;
	Json::FastWriter fastWriter;
	Json::Value context;

	string robotName;
public:
	WMemoryThread(string robotName, const double period) :
			RateThread(int(period * 1000.0)) {
		this->robotName = robotName;
		jsonReader.parse("{\"schemes\":[]}", context);

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

		return true;

	}
	virtual void afterStart(bool s) {
		if (s)
			printf("Working Memory thread started successfully\n");
		else
			printf("Working Memory thread did not start\n");
	}

	virtual void run() {
		printf("waiting for attended context...\n");

		Bottle input;
		Bottle output;
		string input_string;
		attendedContextInputPort.read(input);
		input_string = input.toString();
		prepareInput(input_string);
		jsonReader.parse(input_string.c_str(), context);
		std::cout << "Got context: " << '\n' << context.toStyledString()
				<< '\n';

		printf("writing out attended context...\n");
		output.addString(fastWriter.write(context));
		attendedContextOutputPort.write(output);

		printf("waiting for emotional context...\n");
		input.clear();
		emotionalContextInputPort.read(input);
		input_string = input.toString();
		prepareInput(input_string);
		jsonReader.parse(input_string.c_str(), context);
		std::cout << "Got context: " << '\n' << context.toStyledString()
				<< '\n';
		output.clear();
		printf("writing out emotional context...\n");
				output.addString(fastWriter.write(context));
				emotionalContextOutputPort.write(output);

	}

	virtual void threadRelease() {
		attendedContextInputPort.close();
		attendedContextOutputPort.close();
		emotionalContextInputPort.close();
		emotionalContextOutputPort.close();
	}
};

