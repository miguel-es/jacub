// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <yarp/os/all.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <iostream>

#define CTRL_THREAD_PER     0.02 // [s]
#define MATCH_MODE     "match"
#define UPDATE_MODE     "update"
#include <jutils.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class LTMemoryThread: public RateThread {

	Port modeInputPort;
	Port contextInputPort;
	Port matchThresholdInputPort;
	Port matchedSchemasOutputPort;
	float matchThreshold;

	int totalSchemes;
	bool allowPartialMatch;

	Json::Reader jsonReader;
	//bool accommodated;
	Json::Value kb;

	//Json::Value matchedSchema;

	string robotName;

public:
	LTMemoryThread(string robotName, string kb_path, const double period) :
			RateThread(int(period * 1000.0)) {
		this->robotName = robotName;
		matchThreshold = 100.0;
		allowPartialMatch = false;
		totalSchemes = 0;
		loadKB(kb_path);
		//jsonReader.parse("{\"schemes\":[]}", kb);
	}

	virtual bool threadInit() {

		printf("robot name: %s\n", robotName.c_str());


		if (!modeInputPort.open("/" + robotName + "/memory/mode:i")) {
			printf("Failed creating mode input port for Memory module");
			return false;
		}

		if (!contextInputPort.open("/" + robotName + "/memory/context:i")) {
			printf(
					"Failed creating actual context input port for Memory module");
			return false;
		}

		if (!matchThresholdInputPort.open(
				"/" + robotName + "/memory/matchThreshold:i")) {
			printf(
					"Failed creating match threshold input port for Memory module");
			return false;
		}

		if (!matchedSchemasOutputPort.open(
				"/" + robotName + "/memory/matchedSchemas:o")) {
			printf(
					"Failed creating matched schemas output port for Memory module");
			return false;
		}

		return true;
	}

	virtual void afterStart(bool s) {
		if (s)
			printf("Long-Term Memory thread started successfully\n");
		else
			printf("Long-Term Memory thread did not start\n");

		//t=t0=t1=Time::now();
	}

	virtual void run() {

		printf("waiting for mode selection...\n");

		Bottle input;
		modeInputPort.read(input);
		string input_string;
		if (input.toString() == MATCH_MODE) {
			std::cout << "MATCH_MODE set\n";
			std::cout << "waiting for match threshold...\n";
			matchThresholdInputPort.read(input);
			input_string = input.toString();
			prepareInput(input_string);
			yDebug("Read %s\n",input_string.c_str());

			matchThreshold = std::stof(input_string.c_str());
			std::cout << "got " << matchThreshold << '\n';
			std::cout << "waiting for a context to match...\n";
			contextInputPort.read(input);
			 input_string = input.toString();
			prepareInput(input_string);
			Json::Value context;
			jsonReader.parse(input_string.c_str(), context);
			std::cout << "Got context: " << '\n' << context.toStyledString()
					<< '\n';
			Json::Value matchedSchemas = match(context);
			std::cout << "Matched schemas" << '\n'
					<< matchedSchemas.toStyledString() << '\n';

			std::cout << "writing matched schemas to matchedSchemas:o ...\n";
			Bottle output;
			Json::FastWriter fastWriter;
			output.addString(fastWriter.write(matchedSchemas));
			matchedSchemasOutputPort.write(output);
		}
	}

	virtual void threadRelease() {
		modeInputPort.close();
		contextInputPort.close();
		matchThresholdInputPort.close();
		matchedSchemasOutputPort.close();
	}

private:
	bool loadKB(string kb_file) {
		// Load jacub's knowledge base (schemes)
		std::ifstream ifs;
		ifs.open(kb_file);
		if (!ifs.is_open()) {
			std::cerr << "Could not load knokledge base " << kb_file << "\n";
			return EXIT_FAILURE;
		}

		jsonReader.parse(ifs, kb);

		totalSchemes = kb["schemes"].size();

		std::cout << totalSchemes << " schemas loaded" << '\n';
		return true;
	}
	/**
	 * Compares two contexts
	 *
	 * @param contexts to be compared, the first will be compared agains the second
	 * @return matching percentaje. A 100 return value means a perfect match
	 */

	Json::Value match(Json::Value context) {

		Json::Value matchedSchemas;
		jsonReader.parse("[]", matchedSchemas);
		printf("Probing knowledge base...\n");
		for (Json::Value& schema : kb["schemes"]) {
			if (match(context, schema["context"]) == matchThreshold) {
				matchedSchemas.append(schema);
			}
		}
		return matchedSchemas;
	}

	float match(Json::Value context1, Json::Value context2) {
		//printf("matching...\n");
		float match = 0;

		if (context2[1].size() == 0) // if tactile context is empty then it matches anything
				{
			match += 50.0;
		} else {
			Json::Value::Members contenxtMembers = context1[1].getMemberNames();
			int membersSize = contenxtMembers.size();

			for (string memberName : contenxtMembers) {
				//std::cout << "chekin [ " << memberName <<" ]"<<'\n';
				if (context2[1][memberName].empty()
						|| context1[1][memberName] == context2[1][memberName]) {
					match += 50.0 / membersSize;
				}
			}

			contenxtMembers = context2[1].getMemberNames();
			membersSize = contenxtMembers.size();
			//std::cout << "camate [  ]"<<'\n';
			for (string memberName : contenxtMembers) {
				//std::cout << "chekin [ " << memberName <<" ]"<<'\n';
				if (context1[1][memberName].empty()) {
					match -= 50.0 / membersSize;
				}
			}
		}

		if (context2[0].size() == 0) // if visual context is empty then it matches anything
				{
			match += 50.0;
		} else {
			//printf("cisual contest not empty\n");

			Json::Value::Members contenxtMembers = context2[0].getMemberNames();
			int membersSize = contenxtMembers.size();

			//printf("Miembros => %s",contenxtMembers.toStyledString())
			/*std::cout << "size "<<membersSize<<'\n';
			 float r = (50.0/membersSize);
			 printf("rate %f\n",r);*/

			//std::cout << "camate [  ]"<<'\n';
			for (string memberName : contenxtMembers) {
				//printf( " match [ %f ]\n",match);
				if (!context1[0][memberName].empty()
						&& context1[0][memberName] == context2[0][memberName]) {
					match += 50.0 / membersSize;
				}
			}
			/*
			 contenxtMembers = context2[0].getMemberNames();
			 membersSize = contenxtMembers.size();
			 //std::cout << "camate [  ]"<<'\n';
			 for(string memberName: contenxtMembers)
			 {
			 //std::cout << "chekin [ " << memberName <<" ]"<<'\n';
			 if(context1[0][memberName].empty())
			 {
			 match-=50.0/membersSize;
			 }
			 }*/

		}
		return match;

	}

};
