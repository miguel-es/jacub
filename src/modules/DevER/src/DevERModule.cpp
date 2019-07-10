// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <cmath>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>
#include <yarp/os/all.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <iostream>

#include <jutils.h>
using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class DevERModule: public RFModule {
private:
	Port currentContextInputPort;
	Port output_port;
	Port outputExpectationPort;
	Port ltMemoryModeOutputPort;
	Port currentContextOutputPort;
	Port matchedSchemasInputPort;
	Port actionsOutputPort;
	Port matchThresholdOutputPort;

	int totalSchemes;
	bool allowPartialMatch;

	bool accommodated;
	Json::Value kb;

	Json::Value matchedSchemas;

	string robotName;
	Json::Reader jsonReader;
	Json::FastWriter fastWriter;
public:
	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		//string robotName = rf.find("robot").asString();
		allowPartialMatch = true;
		jsonReader.parse("{\"schemes\":[]}", kb);

		if (!currentContextInputPort.open(
				"/" + robotName + "/DevER/currentContext:i")) {
			yError(
					"Failed creating current context input port for DevER module");
			return false;
		}

		if (!currentContextOutputPort.open(
				"/" + robotName + "/DevER/currentContext:o")) {
			yError(
			"Failed creating current context output port for DevER module");
			return false;
		}

		if (!outputExpectationPort.open(
				"/" + robotName + "/DevER/expectations:o")) {
			yError("Failed creating output port for expectations");
			return false;
		}

		if (!ltMemoryModeOutputPort.open(
				"/" + robotName + "/DevER/memoryMode:o")) {
			yError("Failed creating memory mode output port for DevER module");
			return false;
		}

		if (!matchedSchemasInputPort.open(
				"/" + robotName + "/DevER/matchedSchemas:i")) {
			yError(
			"Failed creating matched schemas input port for DevER module");
			return false;
		}

		if (!actionsOutputPort.open(
				"/" + robotName + "/DevER/actions:o")) {
			yError(
			"Failed creating body action output port for DevER module");
			return false;
		}

		if (!matchThresholdOutputPort.open(
				"/" + robotName + "/DevER/matchThreshold:o")) {
			yError(
			"Failed creating match threshold output port for DevER module");
			return false;
		}

		loadKB(rf.check("kb_file", Value("../../schemas/kb.json")).toString());

		return true;
	}

	virtual bool close() {
		currentContextInputPort.close();
		output_port.close();
		outputExpectationPort.close();
		ltMemoryModeOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {

		std::cout << "DevE-R is in engagement mode\n";

		engagement();

		return true;
	}

private:

	bool loadKB(string kb_file) {
		// Load jacub's knowledge base (schemes)
		//std::ifstream ifs { R"("++")" };
		std::ifstream ifs;
		ifs.open(kb_file);
		//std::ifstream ifs("("+kb_file+")" );
		if (!ifs.is_open()) {
			std::cerr << "Could not load knokledge base " << kb_file << "\n";
			return EXIT_FAILURE;
		}
		jsonReader.parse(ifs, kb);
		totalSchemes = kb["schemes"].size();

		std::cout << totalSchemes << " schemas loaded" << '\n';
		return true;
	}

	bool engagement() {

		Bottle input;
		Bottle output;
		string input_string;

		std::cout << "waiting for an current context...\n";

		currentContextInputPort.read(input);
		Json::Value currentContext;
		std::cout << "read " << input.toString() << '\n';
		input_string = input.toString();
		prepareInput(input_string);

		jsonReader.parse(input_string.c_str(), currentContext);

		std::cout << "Got context: " << '\n' << currentContext.toStyledString()
				<< '\n';

		yDebug(
				" Writing 'match' to %s\n",ltMemoryModeOutputPort.getName().c_str());
		output.addString("match");
		ltMemoryModeOutputPort.write(output);
		output.clear();
		yDebug(
				" Writing out match threshold to %s\n",matchThresholdOutputPort.getName().c_str());
		output.addString("100");
		matchThresholdOutputPort.write(output);
		yDebug(
				" Writing current context to %s \n",currentContextOutputPort.getName().c_str());
		output.clear();
		output.addString(fastWriter.write(currentContext));
		currentContextOutputPort.write(output);

		std::cout << "waiting for matched schemas...\n";

		input.clear();
		matchedSchemasInputPort.read(input);

		input_string = input.toString();
		prepareInput(input_string);
		yDebug(" read %s ",input_string.c_str());

		jsonReader.parse(input_string.c_str(), matchedSchemas);

		std::cout << "Got: " << '\n' << matchedSchemas.toStyledString() << '\n';

		Json::Value selectedSchemas;
		for (Json::Value& schema : matchedSchemas) {//select the best matched schemas

		}
		selectedSchemas = matchedSchemas;
		Json::Value actions;
		jsonReader.parse("[]", actions);

		for (Json::Value& schema : selectedSchemas) {

			for (Json::Value actionv : schema["actions"]) {
				string action = actionv.asString();

				if (action == "showInterestInV" || action == "showInterestInT"
						|| action == "changeAttentionT"
						|| action == "changeAttentionV")  //mental actions
								{
					printf("mental action\n");
				} else   //bodily actions
				{


					actions.append(action);

				}


				/*int i = 0;
				 while(i<200000000){i++;}
				 printf("done action2\n");*/
				Time::delay(2);
			}
		}

		std::cout << "Issuing body actions " << actions.toStyledString()
									<< " to locomotion module\n";
		output.clear();
		output.addString(fastWriter.write(actions));
		actionsOutputPort.write(output);

		return true;
	}

};

