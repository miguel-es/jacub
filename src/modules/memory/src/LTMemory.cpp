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
#define SAVE_MODE	"save"

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


			string ids = matchMode+"=";
			if(matchMode=="exact"){
				ids+="[";
			for(Json::Value schema: matchedSchemas){
				//yDebug("Quesque : %s",schema.toStyledString().c_str());
				for(Json::Value leaf: utils::getLeafs(schema)){
					if(leaf.isMember("match")){
					ids+=" "+leaf["id"].asString();
					}
				}
			}
			ids+="]";
			}else if(matchMode=="partial"){
				ids=" visual:[";
				for(Json::Value schema: matchedSchemas[0]){
					for(Json::Value leaf: utils::getLeafs(schema)){
									if(leaf.isMember("match")){
									ids+=" "+leaf["id"].asString();
									}
								}
							//yDebug("Quesque : %s",schema.toStyledString().c_str());
							//ids+=" "+schema["id"].asString();
						}
				ids+="] tactil:[";
				for(Json::Value schema: matchedSchemas[1]){
										//yDebug("Quesque : %s",schema.toStyledString().c_str());
					for(Json::Value leaf: utils::getLeafs(schema)){
									if(leaf.isMember("match")){
									ids+=" "+leaf["id"].asString();
									}
								}
										//ids+=" "+schema["id"].asString();
									}
				ids+="]";
			}
			yDebug(" DevER: matched schemas %s",ids.c_str());
					yDebug(" LTMemory: matched schemas %s",ids.c_str());

			//yDebug(" LTMemory:  Matched schemas\n %s",matchedSchemas.toStyledString().c_str());

			if(matchMode=="exact")
			yDebug(" LTMemory: %d schemas matched",matchedSchemas.size());
			else
				yDebug(" LTMemory: %d schemas matched visual context and %d matched tactil context ",matchedSchemas[0].size(),matchedSchemas[1].size());

			yDebug(" LTMemory: writing matched schemas to matchedSchemas:o");
			Bottle output;
			Json::FastWriter fastWriter;
			output.addString(fastWriter.write(matchedSchemas));
			matchedSchemasOutputPort.write(output);
		}else if(input.toString() == SAVE_MODE){
			save();
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

		totalSchemes = kb["totalSchemes"].asInt();

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

		bool c1HasVContext = context[0].size()!=0;
		bool c1HasTContext = context[1].size()!=0;

		float vPer = 0;
		float tPer = 0;
		//std::setprecision(3)(333333);
if(c1HasVContext>0){
  vPer = (((float)context[0].size()-1)/(float)context[0].size())*100;
}

		if(c1HasTContext>0){
			  tPer = (((float)context[1].size()-1)/(float)context[1].size())*100;
		}

		for (Json::Value schema : kb["schemes"]) {
			if(matchMode=="partial" && !schema["equilibrated"].asBool()&& !schema.isMember("children")) continue;
			Json::Value leafs = utils::getLeafs(schema);
			//yInfo("LEAFS \n %s",leafs.toStyledString().c_str());
			bool foundMatch = false;
			bool selected = false;
			for (Json::Value& leaf : leafs) {
				yDebug(" LTMemory: testing schema %s",leaf["id"].asString().c_str());
			//int visualContextSize = leaf["context"][0].size();
			//int tactileContextSize = leaf["context"][1].size();

			bool c2HasVContext = leaf["context"][0].size()!=0;
			bool c2HasTContext = leaf["context"][1].size()!=0;
			//bool bothVCEmpty = !c1HasVContext && leaf["context"][0].size()==0;
			//bool bothTCEmpty = !c1HasTContext && leaf["context"][1].size()==0;
			float visualMatch = utils::match(context[0], leaf["context"][0],matchMode);
			float tactilMatch = utils::match(context[1], leaf["context"][1],matchMode);
			//yInfo("vMatch \n %f",visualMatch);
			//yInfo("tMatch \n %f",tactilMatch);



			if(matchMode=="exact" && visualMatch==100 && tactilMatch==100){
				//schema[leaf["id"].asString()]["matched"]=true;
				//leaf["matched"]=true;
			Json::Value jv = utils::markAsMatch(schema,leaf["id"].asString());
yDebug("MArked => id=%s %s",leaf["id"].asString().c_str(), jv.toStyledString().c_str());

				//if(!selected){
				//selected = true;
				matchedSchemas.append(utils::markAsMatch(schema,leaf["id"].asString()));
				//}
				//schema["selected"] = true;
				//break;
			}else if (matchMode=="partial"){
				yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
				yInfo("Matches %f visual and %f tactil, vPer=%f, tPer=%f \n",visualMatch, tactilMatch,vPer,tPer);

				/*if(leaf["context"][0].size()>0 && visualMatch==100 && tactileContextSize==0){ //visual context matches exactly
					matchedSchemas[0].append(schema);

					/*if(!only100){
						jsonReader.parse("[[],[]]", matchedSchemas);
						only100 = true;
					}

break;
				}else */
				if(c1HasTContext && c1HasVContext){

					//if((c2HasVContext && !c2HasTContext && visualMatch==100) || (c2HasVContext && c2HasTContext && visualMatch==100 && tactilMatch!=100)){

									if(c2HasVContext && !c2HasTContext && visualMatch==100){
										matchedSchemas[0].append(utils::markAsMatch(schema,leaf["id"].asString()));
										//break;
									//}else if((c2HasTContext && !c2HasVContext && tactilMatch==100) || (c2HasTContext && c2HasVContext && tactilMatch==100 && visualMatch!=100)){

									}else if(c2HasTContext && !c2HasVContext && tactilMatch==100){
										matchedSchemas[1].append(utils::markAsMatch(schema,leaf["id"].asString()));
																//break;
									}

								}else
									if(c1HasVContext && !c1HasTContext && c2HasVContext && !c2HasTContext && visualMatch==vPer){//visual context differs in one aspect

				//if(c1HasVContext && !c1HasTContext && ((c2HasVContext && !c2HasTContext && visualMatch==vPer)||(c2HasVContext && !c2HasTContext && visualMatch==100.0))){//visual context differs in one aspect

					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
										//schema[leaf["id"].asString()]["matched"]=true;
										//leaf["matched"]=true;
					matchedSchemas[0].append(utils::markAsMatch(schema,leaf["id"].asString()));
					//break;
				}else if(c1HasTContext && !c1HasVContext && c2HasTContext && !c2HasVContext && tactilMatch==tPer){//visual context differs in one aspect
				//}else if(c1HasTContext && !c1HasVContext && ((c2HasTContext && !c2HasVContext && tactilMatch==tPer)||(c2HasTContext && c2HasVContext && tactilMatch==100.0))){//visual context differs in one aspect

					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					//schema[leaf["id"].asString()]["matched"]=true;
					//leaf["matched"]=true;
					matchedSchemas[1].append(utils::markAsMatch(schema,leaf["id"].asString()));
					//break;


				}

				//if(matchMode=="exact" && selected){
				//	yDebug("Appening id=%s",schema["id"].asString().c_str());
				//	matchedSchemas.append(schema);
				//}

				/*else if(leaf["context"][1].size()>0 && visualContextSize==0 && tactilMatch==100){ //tactile context matches exactly
					matchedSchemas[1].append(schema);
					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					/*if(!only100){
						jsonReader.parse("[[],[]]", matchedSchemas);
						only100 = true;
					}

					break;

				}else if(leaf["context"][1].size()>0 && ((tPer>0 && tactilMatch==tPer)|| (tactilMatch==100 && visualMatch!=100 && visualContextSize!=0))){//visual context matches exactly
					//yInfo("Trying schema %s %s\n",schema["id"].asCString(),schema["context"].toStyledString().c_str());
					//yInfo("Matches %f visual and %f tactil \n",visualMatch, tactilMatch);
					matchedSchemas[1].append(schema);
					break;
				}*/
			}
			}
		}
		return matchedSchemas;
	}


	void save() {

		yDebug(" LTMemory: SAVE_MODE set\n");
		Bottle input;
					yDebug(" LTMemory: waiting for new schema to save ...\n");

					//matchModeInputPort.read(input);
					//string input_string = input.toString();

					//yDebug(" LTMemory: waiting for a context to match...\n");
					contextInputPort.read(input);
					string input_string = input.toString();
					prepareInput(input_string);
					Json::Value newSchema;
					jsonReader.parse(input_string.c_str(), newSchema);
					yDebug(" LTMemory: Got schema: %s",newSchema.toStyledString().c_str());
					newSchema["id"]=to_string(++totalSchemes);
					kb["schemes"].append(newSchema);
					kb["totalSchemes"]=totalSchemes;
					yDebug(" LTMemory: Saving new schema: %s",newSchema.toStyledString().c_str());

					//yDebug(" DevER: matched schemas %s",ids.c_str());
						//	yDebug(" LTMemory: matched schemas %s",ids.c_str());
		}
	//}

};
