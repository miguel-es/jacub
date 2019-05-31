#include <string>

/*
* Cleans scape chars from input string to get a well-formed json string
*/
using namespace std;

void prepareInput(string &input){
input.erase(0,1);
            input.erase(input.size()-1,input.size());
            size_t pos;
            //string "\\";

            	while ((pos = input.find("\\n")) != std::string::npos) {
		input.replace(pos, 2, ",");
	}
	while ((pos = input.find("\\")) != std::string::npos) {
		input.replace(pos, 1, "");
	}
}
