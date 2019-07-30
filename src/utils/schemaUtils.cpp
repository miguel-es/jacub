#include <string>
#include <schemaUtils.h>
#include <jsoncpp/json/json.h>

using namespace std;
namespace utils{
float match(Json::Value context1, Json::Value context2){
	if(context1.size()==0 and context2.size()==0) return 100;
	Json::Value::Members contextMembers = context2.getMemberNames();
	int membersSize = contextMembers.size();
	float match = 0;
	for (string memberName : contextMembers) {
					if ((!context1[memberName].empty()
							&& context2[memberName] == "*")
							|| context1[memberName] == context2[memberName]) {
						match += 100.0 / membersSize;
					}
				}
	return match;

}

/**Compares if two context describe de same object
 * Two objects are considered equal if both are of the same color and size
 */
bool areAboutSameObject(Json::Value context1, Json::Value context2){
	if(context1["color"] != context2["color"]) false;
	if(context1["size"] != context2["size"]) false;
	return match;
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



}
