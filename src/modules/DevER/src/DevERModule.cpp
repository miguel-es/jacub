// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <cmath>
#include <queue>

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
	Json::Value selectedSchema;
	Json::Value expectations;
	vector<Json::Value> selectedTreePath;
	Json::Value previousSituation;
	//vector<Json::Value> wm;
	queue<Json::Value> wm; //Working memory (short term memory) TODO: move to Working memory module
	int wmdeep;
	Json::Value currentContext;

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
		jsonReader.parse("[]", expectations);

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

		if (!actionsOutputPort.open("/" + robotName + "/DevER/actions:o")) {
			yError(
			"Failed creating body action output port for DevER module");
			return false;
		}

		if (!matchModeOutputPort.open("/" + robotName + "/DevER/matchMode:o")) {
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
		wmdeep = rf.check("WMDeep", Value("3")).asInt32();
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
		yDebug(" DevER: cycle %d",++cycles);
		yDebug(" DevER: Working memory in  cycle %d: [",cycles);

		/*for (std::vector<Json::Value>::size_type i = 0; i < wm.size(); i++) {
		 yDebug(" t%d = %s",i,wm.at(i).toStyledString().c_str());// << ' ';
		 }*/
		printWM(wm);
		yDebug(" ]");
		if (!engagement()) {
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
			if (schema["equilibrated"] == true) {
				//allow partial patch only if there is at least one equilibrated schema
				allowPartialMatch = true;
				break;
			}
		}
		yDebug(" Allowing partialMatch: %s",allowPartialMatch?"yes":"no");
		if (!allowPartialMatch)
		yDebug(" There are no equilibrated schemas\n");
		cycles = 0;
		return true;
	}

	bool engagement() {
		yDebug(" DevER: In engagement mode\n");
		Json::Value locomotionActions,
		mentalActions;
		Bottle input;
		Bottle output;
		string input_string;

		yDebug(" DevER: waiting for current context...\n");

		currentContextInputPort.read(input);
		//Json::Value currentContext;
		//yDebug(" DevER: read %s \n",input.toString().c_str());
		input_string = input.toString();
		prepareInput(input_string);

		jsonReader.parse(input_string.c_str(), currentContext);
		yDebug(
				" DevER: Got context: %s \n",currentContext.toStyledString().c_str());
		yDebug(" DevER: expected %s \n",expectations.toStyledString().c_str());
		//if(!wm.empty()){
		//wm.front()["expected"]=currentContext;
		//}

		if(!previousSituation.empty()) {
			if(wm.size()>=wmdeep) {

				wm.pop();
			}
			previousSituation["consequence"] = currentContext;
			wm.push(previousSituation);
		}

		if (currentContext.size() == 3
				&& currentContext[2].isMember("boredom")) {
			yDebug(" DevER: boredom detected");
			locomotionActions.append("random");

		} else {

			if(currentContext.size() == 3
					&& (currentContext[2].isMember("joy")||currentContext[2].isMember("surprise"))) {

				accommodation();
			}

			if(expectations.size()!=0 && selectedTreePath.size()>0 && utils::match(currentContext[0],expectations[0],"exact")==100&& utils::match(currentContext[1],expectations[1],"exact")==100) {//expectations fullfilled

				yDebug(" Dev E-R: getting next schema in the tree");
				selectedSchema = selectedTreePath.at(selectedTreePath.size()-1);
				jsonReader.parse("[]", expectations);
				//&& selectedTreePath.size()>0 &&
				if(selectedTreePath.at(selectedTreePath.size()-1).isMember("expected")) {
					expectations.append(selectedTreePath.at(selectedTreePath.size()-1)["expected"][0]);
					expectations.append(selectedTreePath.at(selectedTreePath.size()-1)["expected"][1]);
					/*if(wm.size()>=wmdeep) {
					 wm.pop();
					 }
					 wm.push(selectedTreePath.at(selectedTreePath.size()-1));*/
				}
				selectedTreePath.pop_back();
			} else {
				yDebug(
						" Dev E-R: retrieving schema from memory ");
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
				int r = rand()%2;
				if(allowPartialMatch &&currentContext[1].size()!=0 && currentContext[1]["hand"].asString()=="closed") {
					//if(allowPartialMatch &&  r==0 &&currentContext[1].size()!=0 && currentContext[1]["hand"].asString()=="closed"){
					matchMode = "partial";
				}
				matchMode = "exact";		//TODO:delete

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

				//yDebug(" DevER: Got %s \n",matchedSchemas.toStyledString().c_str());

				//yDebug(" DevER: Got: " << '\n' << matchedSchemas.toStyledString() << '\n';
				string ids = "exact=[";
				for(Json::Value schema: matchedSchemas[0]) {
					//yDebug("Quesque : %s",schema.toStyledString().c_str());
					for(Json::Value leaf: utils::getLeafs(schema)) {
						if(leaf.isMember("match")) {
							ids+=" "+leaf["id"].asString();
						}
					}
				}
				ids+="]";
				ids+=" partial [[";
				for(Json::Value schema: matchedSchemas[1][0]) {
					for(Json::Value leaf: utils::getLeafs(schema)) {
						if(leaf.isMember("match")) {
							ids+=" "+leaf["id"].asString();
						}
					}
					//yDebug("Quesque : %s",schema.toStyledString().c_str());
					//ids+=" "+schema["id"].asString();
				}
				ids+="], [";
				for(Json::Value schema: matchedSchemas[1][1]) {
					//yDebug("Quesque : %s",schema.toStyledString().c_str());
					for(Json::Value leaf: utils::getLeafs(schema)) {
						if(leaf.isMember("match")) {
							ids+=" "+leaf["id"].asString();
						}
					}
					//ids+=" "+schema["id"].asString();
				}
				ids+="]]";

				yDebug(" DevER: matched schemas %s",ids.c_str());
				//jsonReader.parse("[]", selectedSchema);
				// expectations;
				jsonReader.parse("[]", expectations);
				if(matchedSchemas[0].size()>0) {
					//yDebug(" DevER: Getting best exact \n");
					Json::Value bestMatch = getBestMatch(matchedSchemas[0]);
					//selectedSchema.append(bestMatch);
					//if(bestMatch.isMember("expected")){
					//yDebug("getting expectations");
					//expectations.append(getExpectations(bestMatch,0));
					//expectations.append(getExpectations(bestMatch,1));

					//}
					yDebug("getting path");
					selectedTreePath = getPathFromMatchToRoot(bestMatch);
					yDebug("Printing selected branch ");
					for (std::vector<Json::Value>::size_type i = 0; i < selectedTreePath.size(); i++) {
						yDebug(" node %d = %s",i,selectedTreePath.at(i).toStyledString().c_str());	// << ' ';
					}

					selectedSchema = selectedTreePath.at(selectedTreePath.size()-1);
					jsonReader.parse("[]", expectations);
					if(selectedTreePath.at(selectedTreePath.size()-1).isMember("expected")) {
						expectations.append(selectedTreePath.at(selectedTreePath.size()-1)["expected"][0]);
						expectations.append(selectedTreePath.at(selectedTreePath.size()-1)["expected"][1]);

					}

					/*if(selectedTreePath.size()!=0) {
					 wm.push(selectedTreePath.at(selectedTreePath.size()-1));
					 }*/

					selectedTreePath.pop_back();

					if(selectedTreePath.size()>1) {
						Json::Value goal;
						goal["goal"]=selectedTreePath.at(0)["expected"];
						expectations.append(goal);
						//expectations["goal"] = selectedTreePath.at(0)["expected"];
					}

					yDebug(" DevER: expectations from tree: %s \n",expectations.toStyledString().c_str());

				} else if(matchedSchemas[1][0].size()!=0 || matchedSchemas[1][1].size()!=0) {//partial match
					//yDebug(" DevER: Getting best exact else \n");
					selectedTreePath.empty();
					Json::Value mergedSchema;
					Json::Value expectedv;
					Json::Value expectedt;
					jsonReader.parse("{}", expectedv);
					jsonReader.parse("{}", expectedt);
					jsonReader.parse("{\"context\":[],\"actions\":[],\"expected\":[]}", mergedSchema);
					if(matchedSchemas[1][0].size()>0) {
						Json::Value bestMatch = getBestMatch(matchedSchemas[1][0]);
						yDebug("best v");
						mergedSchema["context"].append(bestMatch["context"][0]);
						for(Json::Value action:bestMatch["actions"]) {
							mergedSchema["actions"].append(action);
						}
						yDebug("best v");

						if(bestMatch.isMember("expected")) {
							expectedv = bestMatch["expected"][0];
						}
						yDebug("best v");
						//if(bestMatch.isMember("expected")) expectations.append(bestMatch["expected"][0]);
					}
					if(matchedSchemas[1][1].size()>0) {
						Json::Value bestMatch = getBestMatch(matchedSchemas[1][1]);
						mergedSchema["context"].append(bestMatch["context"][1]);
						yDebug("best t");
						for(Json::Value action:bestMatch["actions"]) {
							mergedSchema["actions"].append(action);
						}
						yDebug("best t");
						jsonReader.parse("{}", expectedt);
						if(bestMatch.isMember("expected")) {
							expectedt = bestMatch["expected"][1];
						}
						yDebug("best t");
					}
					//yDebug("best ttt");
					if(expectedv.size()!=0 || expectedt.size()!=0) {
						//	yDebug("best to");
						mergedSchema["expected"].append(expectedv);
						mergedSchema["expected"].append(expectedt);
						expectations = mergedSchema["expected"];
						Json::Value flag;
						flag["partial"]=true;
						expectations.append(flag);
					} //else expectations = nullptr;

					yDebug("best ti");
					/*if(expectations.size()>0) {
					 Json::Value flag;
					 flag["partial"]=true;
					 expectations.append(flag);
					 }*/
					yDebug("merged");
					selectedSchema=mergedSchema;

				}
			}
			//selectedSchema = matchedSchemas;
			yDebug(" DevER: selectedSchema %s \n",selectedSchema.toStyledString().c_str());
			previousSituation["actions"] = selectedSchema["actions"];
			previousSituation["context"] = currentContext;

			if(selectedSchema.size()==0) {
				yInfo(" DevER: didn't find any applicable schema");
				return false;
			}
			yDebug(" DevER: Saving expectations to memory: %s \n",expectations.toStyledString().c_str());

			//output = expectationsOutputPort.prepare();
			output.clear();
			output.addString(fastWriter.write(expectations));
			expectationsOutputPort.write(output);

			yDebug(" DevER: WRITTEEn");

			jsonReader.parse("[]", locomotionActions);
			//for (Json::Value& schema : selectedSchema) {

			for (Json::Value actionv : selectedSchema["actions"]) {
				string action = actionv.asString();
				yDebug(" DEvER: checking action %s",action.c_str());
				//showInterestInV
				if (action == "showInterestInV" || action == "showInterestInT"
						|| action == "changeAttentionT"
						|| action == "changeAttentionV")//mental actions
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
			//}
		}
		yDebug(
				" DevER: Issuing body actions %s \n",locomotionActions.toStyledString().c_str());

//		yDebug(" DevER: Issuing body actions " << actions.toStyledString()
		//								<< " to locomotion module\n";
		output.clear();
		output.addString(fastWriter.write(locomotionActions));
		actionsOutputPort.write(output);

		yDebug(
				" DevER: Issuing mental actions %s \n",mentalActions.toStyledString().c_str());

		//		yDebug(" DevER: Issuing body actions " << actions.toStyledString()
		//								<< " to locomotion module\n";
		output.clear();
		output.addString(fastWriter.write(mentalActions));
		mentalActionsOutputPort.write(output);

		yDebug(" DevER: Issued");

		return true;
	}

	void reflection() {

		yDebug(" DevER: is in Reflection mode");
		Json::Value locomotionActions;
		jsonReader.parse("[]", locomotionActions);

		vector<std::string> actions {"headUp", "headDown", "headLeft",
			"headRight", "headLeftUp", "headLeftDown", "headRightUp",
			"headRightDown", "handUp", "handDown", "handLeft", "handRight",
			"handBackward", "handForward", "closeHand", "openHand"};

		srand(time(NULL));
		int randomi = rand() % actions.size();
		yDebug(" DevER: reflection random action");
		/*for(int i = 0; i < action.size(); i++)
		 {*/
		float random_index = rand() % actions.size();
		yDebug(" DevER: random seed=%f",random_index);
		string action = actions[random_index];
		//}
		//}

		locomotionActions.append(action);
		Json::Value t;
		jsonReader.parse("{}", t);
		previousSituation["context"]=currentContext;
		previousSituation["actions"]=locomotionActions;

		/*if(wm.size()>=wmdeep) {
		 wm.pop();
		 }
		 wm.push(t);

		 */yDebug(" DevER: Issuing random body actions %s \n",locomotionActions.toStyledString().c_str());

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

		yDebug(" DevER: Issued");
	}

	Json::Value getBestMatch(Json::Value matches) {
		yDebug(" DevER: Getting best match from %s",matches.toStyledString().c_str());
		Json::Value bestMatch;
		if(matches.size()==0) return bestMatch;
		bestMatch = matches[0];
		int emotionalReward = 0;
		if(matches[0].isMember("expected")) {
			emotionalReward = utils::getExpectedEmotionalReward(matches[0]["expected"]);
		}
		int maxEmotionalReward = emotionalReward;
		yDebug(" DevER: expectedEmotionalReward of schema %s is %d",matches[0]["id"].asString().c_str(),emotionalReward);

		int maxH = utils::getSchemaHeight(matches[0]);
		//if(matches.size()==1) return bestMatch;
		int maxProbability = -1;

		for(int i=1;i<matches.size();i++) {
			Json::Value leafs = utils::getLeafs(matches[i]);
			int height = utils::getSchemaHeight(matches[i]);
			int emotionalReward = 0;
			for(Json::Value leaf:leafs) {
				if(!leaf.isMember("match")) continue;
				//yDebug("tik");
				/*if(leaf["context"][0].isMember("distress")){
				 //yDebug("tik2");
				 emotionalReward -= leaf["context"][0]["distress"].asInt();
				 }
				 //yDebug("tik3");
				 if(leaf["context"][0].isMember("contentment")){
				 //yDebug("tik4");
				 emotionalReward += leaf["context"][0]["contentment"].asInt();
				 }*/
				if(leaf.isMember("expected")) {
					emotionalReward = utils::getExpectedEmotionalReward(leaf["expected"]);
				}

				//yDebug("tik5 %s",leaf["actions"].toStyledString().c_str());
				/*for(Json::Value action:leaf["actions"]){
				 if(action.asString()=="showInterestInV"){

				 emotionalReward+=1;
				 }
				 }*/
				int s = rand()%2;
				yDebug(" DevER: rand %d",s);
				yDebug(" DevER: expectedEmotionalReward of schema %s is %d",matches[i]["id"].asString().c_str(),emotionalReward);
				//yDebug("tik6 %s - emotionalReward %d",leaf.toStyledString().c_str(),emotionalReward);
				if(emotionalReward> maxEmotionalReward || height>maxH || (matches[i].isMember("expected") && !bestMatch.isMember("expected"))) {//  leaf.isMember("fulfilled") && matches[i]["fulfilled"].size()>maxProbability&&height>maxH){
					bestMatch = matches[i];
					maxH = height;
					maxEmotionalReward = emotionalReward;
					continue;
				} else if(emotionalReward==maxEmotionalReward && height==maxH && s==1 && (matches[i].isMember("expected") && bestMatch.isMember("expected"))) {
					yDebug(" DevER: random selected %s ",matches[i]["id"].asString().c_str());
					bestMatch = matches[i];
					continue;
				}
			}
		}
		return bestMatch;
	}

	/*Json::Value getExpectations(Json::Value schema,int index) {

	 if(!schema.isMember("children") && !schema.isMember("match")) return nullptr;
	 else if(!schema.isMember("children") && schema.isMember("match")) return schema["expected"][index];
	 if(schema.isMember("children")) {
	 Json::Value leafs = utils::getLeafs(schema);
	 for(Json::Value leaf:leafs) {
	 Json::Value leafExpectations = getExpectations(leaf,index);
	 if(leafExpectations!=nullptr) return leafExpectations;
	 }
	 }
	 return nullptr;
	 }*/

	vector<Json::Value> getPathFromMatchToRoot(Json::Value schema) {
		vector<Json::Value> path;
		if(!schema.isMember("children") && !schema.isMember("match") ) return path;
		else if(!schema.isMember("children") && schema.isMember("match")) {schema.removeMember("match"); path.push_back(schema); return path;}
		if(schema.isMember("children")) {
			Json::Value children = schema["children"];
			schema.removeMember("children");
			path.push_back(schema);
			for(Json::Value child:children) {
				//if(leaf.isMember("match")) {leaf.removeMember("match");leaf.removeMember("children"); path.push_back(leaf);return path;}

				vector<Json::Value> pathFromMatchToChild = getPathFromMatchToRoot(child);
				if(pathFromMatchToChild.size()>0) {
					path.insert(path.end(),pathFromMatchToChild.begin(),pathFromMatchToChild.end());
					return path;
				}
			}
		}
		return path;
	}

	bool accommodation() {

		if(currentContext[2].isMember("joy")) {
			yDebug(" DevER: joy detected!");
			return automatization();
		} else if(currentContext[2].isMember("surprise")) {
			yDebug(" DevER: surprise detected!");
			map<string,string> variables;
			Json::Value newSchema = createSchemaFromWM(wm,&variables);//creates a new schema from the content of working memory
			newSchema["expected"] = currentContext;
			Json::Value deleted;
			newSchema["expected"].removeIndex(2,&deleted);
			yDebug(" DevER: schema from memory %s",newSchema.toStyledString().c_str());
			yDebug(
					" Writing 'save' to %s\n",ltMemoryModeOutputPort.getName().c_str());
			Bottle outputMode;
			outputMode.addString("save");
			ltMemoryModeOutputPort.write(outputMode);

			yDebug(" Dev E-R: writing new schema to ltmemory ");
			Bottle output;
			output.addString(fastWriter.write(newSchema));
			currentContextOutputPort.write(output);

			yDebug(" Dev E-R: done writing ");
			return true;
		}

		return false;
	}

	Json::Value generalization(Json::Value schema) {
		for(string member:schema["context"][0].getMemberNames()) {

			if(member=="contentment" || member=="distress" || member=="color") continue;
			schema["context"][0][member]="*";
			if(schema.isMember("expected")) {
				schema["expected"][0][member]="*";
			}
		}

		for(string member:schema["context"][1].getMemberNames()) {

			if(member=="contentment" || member=="distress") continue;
			schema["context"][1][member]="*";
			if(schema.isMember("expected")) {
				schema["expected"][1][member]="*";
			}
		}

		return schema;
	}
	Json::Value createSchemaFromWM(queue<Json::Value> wm,map<string,string> *variables) {
		yDebug(" Dev E-R: creating new schema");

		Json::Value ancestor;
		if(!wm.empty()) {
			Json::Value node = wm.front();
			wm.pop();
			//yDebug("poped %s",node.toStyledString().c_str());
			//Json::Value deleted;
			Json::Value deleted;
			//yDebug("toing000");
			if(node["consequence"].size()==3) {
				yDebug("toing00'");
				node["consequence"].removeIndex(2,&deleted);
				yDebug("toing00");
			}
			//yDebug("toing0");

			if(node["context"].size()==3) {

				node["context"].removeIndex(2,&deleted);
			}
			Json::Value expected = node["consequence"];
			node.removeMember("consequence");
			node["expected"]=expected;
//yDebug("toing");
			for(string member:node["context"][0].getMemberNames()) {

				if(member=="contentment" || member=="distress") continue;
				if(member=="color") {
					map<string,string>::iterator it;

					yDebug("antes de generalizar color");
					for(map<string, string >::const_iterator it2 = variables->begin();
					    it2 != variables->end(); ++it2)
					{
					    std::cout << it2->first << " " << it2->second << "\n";
					}

					it=variables->find(node["context"][0]["color"].asString());

					if(it!=variables->end()) {
						node["context"][0]["color"]= it->second;
					} else {
						string var = "$"+to_string(variables->size()+1);
						variables->insert(std::pair<string,string>(node["context"][0]["color"].asString(),var));
						node["context"][0]["color"]= var;
						//yDebug("Inserting %s, %s",node["context"][0]["color"].asString().c_str(),("$"+to_string(variables.size()+1)).c_str());
						//variables.insert(std::pair<string,string>(node["context"][0]["color"].asString(),"$"+to_string(variables.size()+1)));

					}
					if(!expected.empty()) {
						it=variables->find(node["expected"][0]["color"].asString());
						if(it!=variables->end()) {
							node["expected"][0]["color"]= it->second;
						} else {
							string var = "$"+to_string(variables->size()+1);
													variables->insert(std::pair<string,string>(node["expected"][0]["color"].asString(),var));
													node["expected"][0]["color"]= var;
						}
					}

					yDebug("antes de generalizar color");
										for(map<string, string >::const_iterator it2 = variables->begin();
										    it2 != variables->end(); ++it2)
										{
										    std::cout << it2->first << " " << it2->second << "\n";
										}

					continue;
				}
				node["context"][0][member]="*";

				node["expected"][0][member]="*";
			}

			for(string member:node["context"][1].getMemberNames()) {

				if(member=="contentment" || member=="distress") continue;
				node["context"][1][member]="*";
				if(node.isMember("expected")) {
					node["expected"][1][member]="*";
				}
			}

			yDebug("generalized %s",node.toStyledString().c_str());

			for(map<string, string >::const_iterator it2 = variables->begin();
														    it2 != variables->end(); ++it2)
														{
														    std::cout << it2->first << " " << it2->second << "\n";
														}

			ancestor = createSchemaFromWM(wm,variables);
			yDebug("returned ancestor %s",ancestor.toStyledString().c_str());
			if(!ancestor.empty()) {

				Json::Value deleted;
				//yDebug("toing000");
				if(ancestor["consequence"].size()==3) {
					yDebug("toing00'");
					ancestor["consequence"].removeIndex(2,&deleted);
					yDebug("toing00");
				}
				//yDebug("toing0");

				if(ancestor["context"].size()==3) {

					ancestor["context"].removeIndex(2,&deleted);
				}
				Json::Value expected = ancestor["consequence"];
				ancestor.removeMember("consequence");
				ancestor["expected"]=expected;
				//yDebug("toing");
				for(string member:ancestor["context"][0].getMemberNames()) {

					if(member=="contentment" || member=="distress") continue;
					if(member=="color") {
						yDebug("Antes de ancestrar con %s, ex %s",ancestor["context"][0]["color"].asString().c_str(),ancestor["expected"][0]["color"].asString().c_str());
						/*for(map<string, string >::const_iterator it2 = variables->begin();
											    it2 != variables->end(); ++it2)
											{
											    std::cout << it2->first << " " << it2->second << "\n";
											}

						map<string,string>::iterator it;
						it=variables->find(ancestor["context"][0]["color"].asString());
						if(it!=variables->end()) {
							ancestor["context"][0]["color"]= it->second;
						} else {
							variables->insert(std::pair<string,string>(ancestor["context"][0]["color"].asString(),"$"+to_string(variables->size()+1)));
							ancestor["context"][0]["color"]= "$"+to_string(variables->size()+1);
						}
						if(!expected.empty()) {
							it=variables->find(ancestor["expected"][0]["color"].asString());
							if(it!=variables->end()) {
								ancestor["expected"][0]["color"]= it->second;
							} else {
								variables->insert(std::pair<string,string>(ancestor["expected"][0]["color"].asString(),"$"+to_string(variables->size()+1)));

								ancestor["expected"][0]["color"]= "$"+to_string(variables->size()+1);
							}
						}
						yDebug("Despues de ancestrar");

						for(map<string, string >::const_iterator it2 = variables->begin();
																	    it2 != variables->end(); ++it2)
																	{
																	    std::cout << it2->first << " " << it2->second << "\n";
								*/								//	}
						continue;
					}
					ancestor["context"][0][member]="*";

					ancestor["expected"][0][member]="*";
				}

				ancestor = utils::appendChild(ancestor,node);

				yDebug("ancestor %s",ancestor.toStyledString().c_str());
				return ancestor;
			} else return node;
		}
		yDebug("hoja");
		return ancestor;
		/*
		 while (!wm.empty()) {
		 Json::Value node;
		 for(Json::Value action: wm.front()["actions"]) {
		 actions.append(action);
		 }
		 wm.pop();
		 }

		 Json::Value actions = getLastActions(wm);

		 Json::Value newSchema;
		 newSchema["context"]=wm.front()["context"];
		 newSchema["expected"]=wm.back()["expected"];
		 newSchema["actions"]=actions;
		 newSchema["equilibrated"]=false;

		 yDebug(" Dev E-R: new schema %s",newSchema.toStyledString().c_str());

		 yDebug(
		 " Writing 'save' to %s\n",ltMemoryModeOutputPort.getName().c_str());
		 Bottle outputMode;
		 outputMode.addString("save");
		 ltMemoryModeOutputPort.write(outputMode);

		 yDebug(" Dev E-R: writing new schema to ltmemory ");
		 Bottle output;
		 output.addString(fastWriter.write(newSchema));
		 currentContextOutputPort.write(output);

		 yDebug(" Dev E-R: done writing ");
		 return true;
		 */}
	bool automatization() {
		yDebug(" Dev E-R: Automating path:");

		Json::Value actions = getLastActions(wm);

		Json::Value newSchema;
		newSchema["context"]=wm.front()["context"];
		newSchema["expected"]=wm.back()["expected"];
		newSchema["actions"]=actions;
		newSchema["equilibrated"]=false;

		yDebug(" Dev E-R: new schema %s",newSchema.toStyledString().c_str());

		yDebug(
				" Writing 'save' to %s\n",ltMemoryModeOutputPort.getName().c_str());
		Bottle outputMode;
		outputMode.addString("save");
		ltMemoryModeOutputPort.write(outputMode);

		yDebug(" Dev E-R: writing new schema to ltmemory ");
		Bottle output;
		output.addString(fastWriter.write(newSchema));
		currentContextOutputPort.write(output);

		yDebug(" Dev E-R: done writing ");
		return true;/*
		 vector<Json::Value> path;
		 if(!schema.isMember("children") && !schema.isMember("match") ) return path;
		 else if(!schema.isMember("children") && schema.isMember("match")) {schema.removeMember("match"); path.push_back(schema); return path;}
		 if(schema.isMember("children")) {
		 Json::Value children = schema["children"];
		 schema.removeMember("children");
		 path.push_back(schema);
		 for(Json::Value child:children) {
		 //if(leaf.isMember("match")) {leaf.removeMember("match");leaf.removeMember("children"); path.push_back(leaf);return path;}

		 vector<Json::Value> pathFromMatchToChild = getPathFromMatchToRoot(child);
		 if(pathFromMatchToChild.size()>0) {
		 path.insert(path.end(),pathFromMatchToChild.begin(),pathFromMatchToChild.end());
		 return path;
		 }
		 }
		 }
		 return path;*/
	}

	void printWM(queue<Json::Value> wm)
	{
		//printing content of queue
		//string content ="[";
		int i = 0;
		while (!wm.empty()) {
			//content+=wm.front().toStyledString();
			yDebug(" DevER: t%d %s",i++,wm.front().toStyledString().c_str());
			wm.pop();

		}
		//content+="]";
		//yDebug(" DevER: Working memory content %s",content.c_str());
	}

	Json::Value getLastActions(queue<Json::Value> wm)
	{
		//printing content of queue
		Json::Value actions;
		jsonReader.parse("[]", actions);
		//Json::Value actions;
		while (!wm.empty()) {
			for(Json::Value action: wm.front()["actions"]) {
				actions.append(action);
			}
			wm.pop();
		}
		return actions;
		//content+="]";
		//yDebug(" DevER: Working memory content %s",content);
	}
};

