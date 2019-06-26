// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <cmath>

#include <jutils.cpp>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/all.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <iostream>

#define CTRL_THREAD_PER     0.02 // [s]
#define MATCH_MODE     "match"
#define UPDATE_MODE     "update"

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class LTMemoryModule: public RFModule {
protected:
	//LTMemoryThread *ltMemoryThr;
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
	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		printf("robot name: %s\n", robotName.c_str());
		matchThreshold = 100.0;
		jsonReader.parse("{\"schemes\":[]}", kb);

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
				"/" + robotName + "/memory/matchThreashold:i")) {
			printf(
					"Failed creating match threashold input port for Memory module");
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

	virtual bool close() {

		 modeInputPort.close();
		 contextInputPort.close();
		 matchThresholdInputPort.close();
		 matchedSchemasOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {

		printf("waiting for mode selection...\n");

		Bottle input;
		modeInputPort.read(input);
		if (input.toString() == MATCH_MODE) {
			std::cout << "MATCH_MODE set\n";
			std::cout << "waiting for match threshold...\n";
			matchThresholdInputPort.read(input);
			matchThreshold = std::stof(input.toString().c_str());
			std::cout << "got " << matchThreshold << '\n';
			std::cout << "waiting for a context to match...\n";
			contextInputPort.read(input);
			string input_string = input.toString();
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

		return true;
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

int main(int argc, char *argv[]) {
	Network yarp;

	if (!yarp.checkNetwork()) {
		yError("Yarp server does not seem available\n");
		return 1;
	}

	LTMemoryModule ltMemory;

	ResourceFinder rf;
	rf.setVerbose(); //logs searched directories
	rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
	rf.configure(argc, argv);

	ltMemory.runModule(rf);
}

