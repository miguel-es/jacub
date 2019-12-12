/*
* Cleans scape chars from input string to get a well-formed json string
*/

/*#ifndef prepareInput
#define prepareInput*/

#include <jsoncpp/json/json.h>
#include <string>
#ifndef SCHEMA_FUNCTIONS
#define SCHEMA_FUNCTIONS
namespace utils{
float match(Json::Value context1, Json::Value context2,std::string matchMode);

bool areAboutSameObject(Json::Value context1, Json::Value context2);

Json::Value getLeafs(Json::Value schema);
int getSchemaHeight(Json::Value schema);
int getExpectedEmotionalReward(Json::Value schema);
Json::Value markAsMatch(Json::Value schema, std::string id);
Json::Value appendChild(Json::Value schema, Json::Value node);
}



#endif




//#endif
