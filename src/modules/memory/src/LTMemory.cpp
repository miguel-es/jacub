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
#include <schemaUtils.h>

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class LTMemoryThread: public RateThread {

	Port modeInputPort;
	Port contextInputPort;
	Port matchModeInputPort;
	Port matchedSchemasOutputPort;
	string matchMode;

	Bottle input;
	int totalSchemes;
	bool allowPartialMatch;

	Json::Reader jsonReader;
	//bool accommodated;
	Json::Value kb;
	int cycles;

	//Json::Value matchedSchema;

	string robotName;

public:
	LTMemoryThread(string robotName, string kb_path, const double period) :
			RateThread(int(period * 1000.0)) {
		this->robotName = robotName;
		matchMode = 100.0;
		allowPartialMatch = false;
		totalSchemes = 0;
		cycles = 0;
		loadKB(kb_path);
		//jsonReader.parse("{\"schemes\":[]}", kb);
	}

	virtual bool threadInit() {

		yInfo("robot name: %s\n", robotName.c_str());

		if (!modeInputPort.open("/" + robotName + "/memory/mode:i")) {
			yInfo("Failed creating mode input port for Memory module");
			return false;
		}

		if (!contextInputPort.open("/" + robotName + "/memory/context:i")) {
			yInfo(
					"Failed creating actual context input port for Memory module");
			return false;
		}

		if (!matchModeInputPort.open("/" + robotName + "/memory/matchMode:i")) {
			yInfo(
					"Failed creating match threshold input port for Memory module");
			return false;
		}

		if (!matchedSchemasOutputPort.open(
				"/" + robotName + "/memory/matchedSchemas:o")) {
			yInfo(
					"Failed creating matched schemas output port for Memory module");
			return false;
		}

		return true;
	}

	virtual void afterStart(bool s) {
		if (s)
			yInfo("Long-Term Memory thread started successfully\n");
		else
			yInfo("Long-Term Memory thread did not start\n");

		//t=t0=t1=Time::now();
	}

	virtual void run() {
		yDebug(" LTMemory: cycle %d",++cycles);

		yDebug(" LTMemory: waiting for mode selection...\n");

		modeInputPort.read(input);
		//yDebug(" LTMMemory: Read");
		string input_string;
		if (input.toString() == MATCH_MODE) {
			yDebug(" LTMemory: MATCH_MODE set\n");
			yDebug(" LTMemory: waiting for match mode...\n");
			Bottle input;
			matchModeInputPort.read(input);
			input_string = input.toString();
			//prepareInput(input_string);
			//yDebug(" LTMemory:Read %s\n",input_string.c_str());

			matchMode = input_string.c_str();
			yDebug(" LTMemory: got %s",matchMode.c_str());
			yDebug(" LTMemory: waiting for a context to match...\n");
			contextInputPort.read(input);
			input_string = input.toString();
			prepareInput(input_string);
			Json::Value context;
			jsonReader.parse(input_string.c_str(), context);
			yDebug(" LTMemory: Got context: %s",context.toStyledString().c_str());
			Json::Value matchedSchemas = match(context);
			yDebug(" LTMemory:  Matched schemas\n %s",matchedSchemas.toStyledString().c_str());

			if(matchMode=="exact")
			yDebug(" LTMemory: %d schemas matched",matchedSchemas.size());
			else
				yDebug(" LTMemory: %d schemas matched visual context and %d matched tactil context ",matchedSchemas[0].size(),matchedSchemas[1].size());

			yDebug(" LTMemory: writing matched schemas to matchedSchemas:o");
			Bottle output;
			Json::FastWriter fastWriter;
			output.addString(fastWriter.write(matchedSchemas));
			matchedSchemasOutputPort.write(output);
		}
	}

	virtual void threadRelease() {
		//modeInputPort.close();
		contextInputPort.close();
		matchModeInputPort.close();
		matchedSchemasOutputPort.close();
	}

private:
	bool loadKB(string kb_file) {
		// Load jacub's knowledge base (schemes)
		std::ifstream ifs;
		ifs.open(kb_file);
		if (!ifs.is_open()) {
			std::cerr << "Could not load knowledge base " << kb_file << "\n";
			return EXIT_FAILURE;
		}

		jsonReader.parse(ifs, kb);

		/*for (Json::Value& schema : kb["schemes"]) {
					if(schema["equilibrated"]==true){
						//allow partial patch only if there is at least one equilibrated schema
						allowPartialMatch = true;
					break;
					}
				}
		yInfo("LTM: Allowing partialMatches %b",allowPartialMatch);
		if(!allowPartialMatch){
		yInfo("There are no equilibrated schemas\n");
		}*/

		totalSchemes = kb["schemes"].size();

		yDebug(" LTMemory: %d schemas loaded",totalSchemes);
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
		if(matchMode=="exact")
		jsonReader.parse("[]", matchedSchemas);
		else
			jsonReader.parse("[[],[]]", matchedSchemas);
		// if partial match is set, the result is a list of two lists
		// one for schemas matching visual and one for tactile
		bool only100 = false;
		yInfo("Probing knowledge base with match mode set to %s...\n",
				matchMode.c_str());

		for (Json::Value& schema : kb["schemes"]) {
			if(matchMode=="partial" && !schema["equilibrated"].asBool()) continue;
			Json::Value leafs = utils::getLeafs(schema);
			//yInfo("LEAFS \n %s",leafs.toStyledString().c_str());
			bool foundMatch = false;
			for (Json::Value& leaf : leafs) {
			int visualContextSize = leaf["context"][0].size();
			int tactileContextSize = leaf["context"][1].size();

			float visualMatch = utils::match(context[0], leaf["context"][0]);
			float tactilMatch = utils::match(context[1], leaf["context"][1]);
			//yInfo("vMatch \n %f",visualMatch);
			//yInfo("tMatch \n %f",tactilMatch);
			float vPer = 0;
			float tPer = 0;
if(visualContextSize>0){
	  vPer = (((float)visualContextSize-1)/(float)visualContextSize)*100;
}

			if(tactileContextSize>0){
				  tPer = (((float)tactileContextSize-1)/(float)tactileContextSize)*100;
			}


			if(matchMode=="exact" && visualMatch==100 && tactilMatch==100){
				matchedSchemas.append(schema);
				break;
			}else if (matchMode=="partial"){
				yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
				yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);

				if(leaf["context"][0].size()>0 && visualMatch==100 && tactileContextSize==0){ //visual context matches exactly
					matchedSchemas[0].append(schema);

					/*if(!only100){
						jsonReader.parse("[[],[]]", matchedSchemas);
						only100 = true;
					}

break;*/
				}else if(!only100  && vPer>0 && visualMatch==vPer && tactileContextSize==0){//visual context differs in one aspect
					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					matchedSchemas[0].append(schema);
					break;
				}

				else if(leaf["context"][1].size()>0 && visualContextSize==0 && tactilMatch==100){ //tactile context matches exactly
					matchedSchemas[1].append(schema);
					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					/*if(!only100){
						jsonReader.parse("[[],[]]", matchedSchemas);
						only100 = true;
					}

					break;*/

				}else if(!only100 && tPer>0 && visualContextSize==0 && tactilMatch==tPer){//visual context matches exactly
					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					matchedSchemas[1].append(schema);
					break;
				}
			}
			}
		}
		return matchedSchemas;
	}



	/*float match(Json::Value context1, Json::Value context2) {
			bool matchesVisual = true;
			bool matchesTactile = true;
			Json::Value::Members contextMembers = context2[0].getMemberNames();
			for (string memberName : contextMembers) {
				if ((context2[0][memberName].empty()
						&& context2[0][memberName] == "*")
						|| context1[0][memberName] != context2[0][memberName]) {
					matchesVisual = false;
				}
			}

			Json::Value::Members contextMembers = context2[1].getMemberNames();
			for (string memberName : contextMembers) {
				if ((context2[1][memberName].empty()
						&& context2[1][memberName] == "*")
						|| context1[1][memberName] != context2[1][memberName]) {
					matchesTactile = false;
				}
			}


		//}
			if(matchMode=="partial"){
				return (matchesVisual && !matchesTactile) || (!matchesVisual && matchesTactile);
			}else{
				return matchesVisual && matchesTactile;
			}
		float match = 0;

		if (context2[1].size() == 0) // if tactile context is empty then it matches anything
				{
			match += 50.0;
		} else {
			Json::Value::Members contenxtMembers = context1[1].getMemberNames();
			int membersSize = contenxtMembers.size();

			for (string memberName : contenxtMembers) {
				//yDebug(" LTMemory: chekin [ " << memberName <<" ]"<<'\n';
				if (context2[1][memberName].empty()
						|| context1[1][memberName] == context2[1][memberName]) {
					match += 50.0 / membersSize;
				}
			}

			contenxtMembers = context2[1].getMemberNames();
			membersSize = contenxtMembers.size();
			//yDebug(" LTMemory: camate [  ]"<<'\n';
			for (string memberName : contenxtMembers) {
				//yDebug(" LTMemory: chekin [ " << memberName <<" ]"<<'\n';
				if (context1[1][memberName].empty()) {
					match -= 50.0 / membersSize;
				}
			}
		}

		if (context2[0].size() == 0) // if visual context is empty then it matches anything
				{
			match += 50.0;
		} else {
			//yInfo("cisual contest not empty\n");

			Json::Value::Members contenxtMembers = context2[0].getMemberNames();
			int membersSize = contenxtMembers.size();

			//yInfo("Miembros => %s",contenxtMembers.toStyledString())
			/*yDebug(" LTMemory: size "<<membersSize<<'\n';
			 float r = (50.0/membersSize);
			 yInfo("rate %f\n",r);*/

			//yDebug(" LTMemory: camate [  ]"<<'\n';
			/*for (string memberName : contenxtMembers) {
				//yInfo( " match [ %f ]\n",match);
				if (!context1[0][memberName].empty()
						&& context1[0][memberName] == context2[0][memberName]) {
					match += 50.0 / membersSize;
				}
			}
			/*
			 contenxtMembers = context2[0].getMemberNames();
			 membersSize = contenxtMembers.size();
			 //yDebug(" LTMemory: camate [  ]"<<'\n';
			 for(string memberName: contenxtMembers)
			 {
			 //yDebug(" LTMemory: chekin [ " << memberName <<" ]"<<'\n';
			 if(context1[0][memberName].empty())
			 {
			 match-=50.0/membersSize;
			 }
			 }*/

		//}
		//return match;

	//}

};
