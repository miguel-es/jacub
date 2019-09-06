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
#include <schemaUtils.h>
using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class DevERModule: public RFModule {
private:
	Port currentContextInputPort;
	Port output_port;
	Port ltMemoryModeOutputPort;
	Port currentContextOutputPort;
	Port matchedSchemasInputPort;
	Port actionsOutputPort;
	Port matchModeOutputPort;
	Port expectationsOutputPort;
	Port mentalActionsOutputPort;
	//BufferedPort<Bottle> expectationsOutputPort;

	bool allowPartialMatch;

	bool accommodated;
	Json::Value kb;

	//Json::Value matchedSchemas;
	Json::Value selectedSchemas;

	string robotName;
	Json::Reader jsonReader;
	Json::FastWriter fastWriter;
	int cycles;
public:
	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		//string robotName = rf.find("robot").asString();
		allowPartialMatch = false; //TODO: check if kb has stabilished schemas


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

		if (!matchModeOutputPort.open(
				"/" + robotName + "/DevER/matchMode:o")) {
			yError(
			"Failed creating match mode output port for DevER module");
			return false;
		}
		if (!expectationsOutputPort.open(
						"/" + robotName + "/DevER/expectations:o")) {
					yError(
					"Failed creating match mode output port for DevER module");
					return false;
		}
		if (!mentalActionsOutputPort.open(
								"/" + robotName + "/DevER/mentalActions:o")) {
							yError(
							"Failed creating mental actions output port for DevER module");
							return false;
				}

		//Probe kb for equilibrated schemas TODO: this shoud be asked to the memory module
		checkKB(rf.check("kb", Value("")).toString());
		//loadKB(rf.check("kb_file", Value("../../schemas/kb.json")).toString());


		return true;
	}

	virtual bool close() {
		currentContextInputPort.close();
		output_port.close();

		ltMemoryModeOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		if(!engagement()){
			reflection();
		}

		return true;
	}

private:

	bool checkKB(string kb_file) {
		// Load jacub's knowledge base (schemes)
		//std::ifstream ifs { R"("++")" };
		std::ifstream ifs;
		ifs.open(kb_file);
		//std::ifstream ifs("("+kb_file+")" );
		if (!ifs.is_open()) {
			std::cerr << "Could not check knokledge base " << kb_file << "\n";
			return EXIT_FAILURE;
		}
		jsonReader.parse(ifs, kb);

		for (Json::Value& schema : kb["schemes"]) {
			if(schema["equilibrated"]==true){
				//allow partial patch only if there is at least one equilibrated schema
				allowPartialMatch = true;
			break;
			}
		}
		(" Allowing partialMatch: %s",allowPartialMatch?"yes":"no");
		if(!allowPartialMatch) (" There are no equilibrated schemas\n");
cycles = 0;
		return true;
	}

	bool engagement() {
		yDebug(" DevER: cycle %d",++cycles);
		yDebug(" DevER: In engagement mode\n");

		Bottle input;
		Bottle output;
		string input_string;

		yDebug(" DevER: waiting for current context...\n");

		currentContextInputPort.read(input);
		Json::Value currentContext;
		//yDebug(" DevER: read %s \n",input.toString().c_str());
		input_string = input.toString();
		prepareInput(input_string);

		jsonReader.parse(input_string.c_str(), currentContext);

		yDebug(" DevER: Got context: %s \n",currentContext.toStyledString().c_str());

		yDebug(
				" Writing 'match' to %s\n",ltMemoryModeOutputPort.getName().c_str());
		Bottle outputMode;
		outputMode.addString("match");
		ltMemoryModeOutputPort.write(outputMode);

		output.clear();
		yDebug(
				" Writing out match mode to %s\n",matchModeOutputPort.getName().c_str());
		string matchMode = "exact";
		srand(time(NULL));
		int r = rand();
		if(allowPartialMatch &&  r ==0){
			matchMode = "partial";
		}
		matchMode = "partial";//TODO:delete

Bottle outputM;
		outputM.addString(matchMode);
		matchModeOutputPort.write(outputM);
		//yDebug(" DevER: wrote match mode %s random = %d\n",outputM.toString().c_str(),r);
		yDebug(
				" Writing current context to %s \n",currentContextOutputPort.getName().c_str());
		output.clear();
		output.addString(fastWriter.write(currentContext));
		currentContextOutputPort.write(output);

		yDebug(" DevER: waiting for matched schemas...\n");

		input.clear();
		matchedSchemasInputPort.read(input);

		input_string = input.toString();
		prepareInput(input_string);
		//yDebug(" DevER: read %s ",input_string.c_str());
Json::Value matchedSchemas;
		jsonReader.parse(input_string.c_str(), matchedSchemas);

		yDebug(" DevER: Got %s \n",matchedSchemas.toStyledString().c_str());

		//yDebug(" DevER: Got: " << '\n' << matchedSchemas.toStyledString() << '\n';
		string ids = "";
		if(matchedSchemas.size()==0){
		for(Json::Value schema: matchedSchemas){
			//yDebug("Quesque : %s",schema.toStyledString().c_str());
			ids+=" "+schema["id"].asString();
		}
		}else if(matchedSchemas.size()==2){
			for(Json::Value schema: matchedSchemas[0]){
						//yDebug("Quesque : %s",schema.toStyledString().c_str());
						ids+=" "+schema["id"].asString();
					}
			ids+=" - ";
			for(Json::Value schema: matchedSchemas[1]){
									//yDebug("Quesque : %s",schema.toStyledString().c_str());
									ids+=" "+schema["id"].asString();
								}
		}
		yDebug(" DevER: matched schemas [%s]",ids.c_str());
		jsonReader.parse("[]", selectedSchemas);
		Json::Value expectations;
		jsonReader.parse("[]", expectations);
		if(matchMode=="exact" && matchedSchemas.size()>0){
			//yDebug(" DevER: Getting best exact \n");
			Json::Value bestMatch = getBestMatch(matchedSchemas);
			selectedSchemas.append(bestMatch);
			if(bestMatch.isMember("expected")) expectations = bestMatch["expected"];
		}else{
			//yDebug(" DevER: Getting best exact else \n");
			if(matchedSchemas[0].size()>0 && matchedSchemas[0].size()>0){
				Json::Value bestMatch = getBestMatch(matchedSchemas[0]);
				selectedSchemas.append(bestMatch);
				if(bestMatch.isMember("expected")) expectations.append(bestMatch["expected"][0]);
			}
			if(matchedSchemas[0].size()>0 && matchedSchemas[1].size()>0){
							Json::Value bestMatch = getBestMatch(matchedSchemas[1]);
							selectedSchemas.append(bestMatch);
							if(bestMatch.isMember("expected")) expectations.append(bestMatch["expected"][1]);
						}
		}
		//selectedSchemas = matchedSchemas;
		yDebug(" DevER: selectedSchemas %s \n",selectedSchemas.toStyledString().c_str());
		if(selectedSchemas.size()==0){
			yInfo(" DevER: didn't find any applicable schema");
			return false;
		}
		yDebug(" DevER: Saving expectations to memory: %s \n",expectations.toStyledString().c_str());

		//output = expectationsOutputPort.prepare();
		output.clear();
		output.addString(fastWriter.write(expectations));
		expectationsOutputPort.write(output);

	//yDebug(" DevER: WRITTEEn");
		Json::Value locomotionActions,mentalActions;
		jsonReader.parse("[]", locomotionActions);
		for (Json::Value& schema : selectedSchemas) {

			for (Json::Value actionv : schema["actions"]) {
				string action = actionv.asString();
				if (action == "showInterestInV" || action == "showInterestInT"
						|| action == "changeAttentionT"
						|| action == "changeAttentionV")  //mental actions
				{
mentalActions.append(action);
		//			yDebug(" DevER: mental action\n");
				} else   //bodily actions
				{


					locomotionActions.append(action);

				}


				/*int i = 0;
				 while(i<200000000){i++;}
				 yDebug(" DevER: done action2\n");*/
				Time::delay(2);
			}
		}
		yDebug(" DevER: Issuing body actions %s \n",locomotionActions.toStyledString().c_str());

//		yDebug(" DevER: Issuing body actions " << actions.toStyledString()
	//								<< " to locomotion module\n";
		output.clear();
		output.addString(fastWriter.write(locomotionActions));
		actionsOutputPort.write(output);

		yDebug(" DevER: Issuing mental actions %s \n",mentalActions.toStyledString().c_str());

		//		yDebug(" DevER: Issuing body actions " << actions.toStyledString()
			//								<< " to locomotion module\n";
				output.clear();
				output.addString(fastWriter.write(mentalActions));
				mentalActionsOutputPort.write(output);



//yDebug(" DevER: Issued");

		return true;
	}

	void reflection(){

		yDebug(" DevER: is in Reflection mode");
			Json::Value locomotionActions;
			jsonReader.parse("[]", locomotionActions);
			locomotionActions.append("random");

			yDebug(" DevER: Issuing random body actions %s \n",locomotionActions.toStyledString().c_str());

			Bottle output;
			//output.clear();
			output.addString(fastWriter.write(locomotionActions));
			actionsOutputPort.write(output);

			Json::Value mentalActions;
						jsonReader.parse("[]", mentalActions);
			//yDebug(" DevER: Issuing mental actions %s \n",mentalActions.toStyledString().c_str());

			//		yDebug(" DevER: Issuing body actions " << actions.toStyledString()
				//								<< " to locomotion module\n";
					output.clear();
					output.addString(fastWriter.write(mentalActions));
					mentalActionsOutputPort.write(output);



	//yDebug(" DevER: Issued");
	}

	Json::Value getBestMatch(Json::Value matches){
		yDebug(" DevER: Getting best match");
		Json::Value bestMatch;
		if(matches.size()==0) return bestMatch;
		bestMatch = matches[0];
		int maxEmotionalResponse = 0;
		int maxH = utils::getSchemaHeight(matches[0]);
		if(matches.size()==1) return bestMatch;
		int maxProbability = -1;

		for(int i=1;i<matches.size();i++){
			Json::Value leafs = utils::getLeafs(matches[i]);
			int height = utils::getSchemaHeight(matches[i]);
			int emotionalResponse = 0;
			for(Json::Value leaf:leafs){
				//yDebug("tik");
				if(leaf["context"][0].isMember("distress")){
					//yDebug("tik2");
					emotionalResponse -= leaf["context"][0]["distress"].asInt();
				}
				//yDebug("tik3");
				if(leaf["context"][0].isMember("contentment")){
					//yDebug("tik4");
					emotionalResponse += leaf["context"][0]["contentment"].asInt();
				}

				//yDebug("tik5 %s",leaf["actions"].toStyledString().c_str());
				for(Json::Value action:leaf["actions"]){
					if(action.asString()=="showInterestInV"){

					emotionalResponse+=1;
					}
				}
				//yDebug("tik6 %s - emotionalResponse %d",leaf.toStyledString().c_str(),emotionalResponse);
			if(emotionalResponse> maxEmotionalResponse){//  leaf.isMember("fulfilled") && matches[i]["fulfilled"].size()>maxProbability&&height>maxH){
				bestMatch = matches[i];
				maxH = height;
				maxEmotionalResponse = emotionalResponse;
				continue;
			}
			}
		}
		return bestMatch;
	}

};

