/*
* Cleans scape chars from input string to get a well-formed json string
*/

/*#ifndef prepareInput
#define prepareInput*/

#include <jsoncpp/json/json.h>
#ifndef SCHEMA_FUNCTIONS
#define SCHEMA_FUNCTIONS
namespace utils{
float match(Json::Value context1, Json::Value context2);

bool areAboutSameObject(Json::Value context1, Json::Value context2);

Json::Value getLeafs(Json::Value schema);
int getSchemaHeight(Json::Value schema);
int getExpectedEmotionalReward(Json::Value schema);
}
#endif




//#endif
