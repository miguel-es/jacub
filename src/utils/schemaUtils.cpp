#include <string>
#include <schemaUtils.h>
#include <jsoncpp/json/json.h>
#include <yarp/os/impl/Logger.h>


using namespace std;
namespace utils{
float match(Json::Value context1, Json::Value context2){
	if(context1.size()==0 and context2.size()==0) return 100;
	Json::Value::Members context1Members = context1.getMemberNames();
	Json::Value::Members contextMembers = context2.getMemberNames();
	int membersSize = contextMembers.size();
	float match = 0;
	for (string memberName : contextMembers) {
		if(context1[memberName].empty()) return 0;
					if (context2[memberName] == "*"
							|| context1[memberName] == context2[memberName]) {
						//match += 100.0 / membersSize;
						match+=1.0;
					}
				}
	return (match/membersSize)*100;

}

/**Compares if two context describe de same object
 * Two objects are considered equal if both are of the same color and size
 */
bool areAboutSameObject(Json::Value context1, Json::Value context2){
	//yDebug("comparig obj1 %s y obj2 %s",context1.toStyledString().c_str(),context2.toStyledString().c_str());
	int equiality = context1["color"].asString().compare(context2["color"].asString());


	return equiality==0;
	//if(context1["size"] != context2["size"]) false;
	//return true;
}

/**
 * Gets all the leafs in the schema tree
 *
 * @param schema to be explored
 * @return json list containing the leafs
 */

Json::Value getLeafs(Json::Value schema){
	Json::Value leafs;
	Json::Reader jsonReader;
	jsonReader.parse("[]", leafs);
	if(!schema.isMember("children")){
		leafs.append(schema);
		return leafs;
	}

	for (Json::Value& child : schema["children"]) {
		Json::Value childLeafs = getLeafs(child);
		for (Json::Value& leaf : childLeafs) {
			leafs.append(leaf);
	}
	}
	return leafs;


}

int getSchemaHeight(Json::Value schema){

	if(!schema.isMember("children")){
			return 1;
		}
	int childHMax = 0;
	for (Json::Value& child : schema["children"]) {
		int childHeight = getSchemaHeight(child);
		if(childHeight>childHMax)
			childHMax = childHeight;
	}
	return 1+childHMax;
}

int getExpectedEmotionalReward(Json::Value schema){
	int emotionalReward = 0;
	//yDebug(" Getting expected emotional reward of schem%s",context.toStyledString().c_str());

	if(schema.isMember("expected")){

	if(schema["expected"][0].isMember("distress")){
		//yDebug("tik2");
		emotionalReward -= schema["expected"][0]["distress"].asInt();
	}
	//yDebug("tik3");
	if(schema["expected"][0].isMember("contentment")){
		//yDebug("tik4");
		emotionalReward += schema["expected"][0]["contentment"].asInt();
	}

	}else{
		return 0;
		/*for(Json::Value action:leaf["actions"]){
							if(action.asString()=="showInterestInV"){

							emotionalReward+=1;
							}
						}*/
	}

	//yDebug("tik5 %s",leaf["actions"].toStyledString().c_str());
	return emotionalReward;

}



}
