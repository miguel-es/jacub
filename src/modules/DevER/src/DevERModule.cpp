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

	int totalSchemes;
	bool allowPartialMatch;

	bool accommodated;
	Json::Value kb;

	//Json::Value matchedSchemas;
	Json::Value selectedSchemas;

	string robotName;
	Json::Reader jsonReader;
	Json::FastWriter fastWriter;
public:
	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		//string robotName = rf.find("robot").asString();
		allowPartialMatch = true; //TODO: check if kb has stabilished schemas
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
				" Writing out match mode to %s\n",matchModeOutputPort.getName().c_str());
		string matchMode = "exact";
		srand(time(NULL));
		int r = rand();
		if(allowPartialMatch &&  r ==0){
			matchMode = "partial";
		}
		matchMode = "exact";//TODO:delete
		yDebug(
						" Match mode %s random = %d\n",matchMode.c_str(),r);

		output.addString(matchMode);
		matchModeOutputPort.write(output);
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
Json::Value matchedSchemas;
		jsonReader.parse(input_string.c_str(), matchedSchemas);

		std::cout << "Got: " << '\n' << matchedSchemas.toStyledString() << '\n';

		jsonReader.parse("[]", selectedSchemas);
		Json::Value expectations;
		jsonReader.parse("[]", expectations);
		if(matchMode=="exact" && matchedSchemas.size()>0){

			Json::Value bestMatch = getBestMatch(matchedSchemas);
			selectedSchemas.append(bestMatch);
			if(bestMatch.isMember("expected")) expectations = bestMatch["expected"];
		}else{
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
		printf("selectedSchemas %s \n",selectedSchemas.toStyledString().c_str());
		printf("Saving expectations to memory: %s \n",expectations.toStyledString().c_str());

		output.clear();
		output.addString(fastWriter.write(expectations));
		expectationsOutputPort.write(output);

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

	Json::Value getBestMatch(Json::Value matches){
		printf("Getting best match");
		Json::Value bestMatch;
		if(matches.size()==0) return bestMatch;
		bestMatch = matches[0];
		int maxH = utils::getSchemaHeight(matches[0]);
		if(matches.size()==1) return bestMatch;
		int maxProbability = -1;
		for(int i=1;i<matches.size();i++){
			Json::Value leafs = utils::getLeafs(matches[i]);
			int height = utils::getSchemaHeight(matches[i]);
			for(Json::Value leaf:leafs){
			if(leaf.isMember("fulfilled") && matches[i]["fulfilled"].size()>maxProbability&&height>maxH){
				bestMatch = matches[i];
				maxH = height;
				continue;
			}
			}
		}
		return bestMatch;
	}

};

