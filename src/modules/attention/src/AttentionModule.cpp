// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
 * @ingroup icub_tutorials
 * \defgroup imageProc imageProc
 */

#include <stdio.h>
#include <iostream>
#include <yarp/os/Network.h>
#include <yarp/os/Time.h>
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/BufferedPort.h>
#include <string>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <yarp/os/impl/Logger.h>
#include <jutils.h>
#include <schemaUtils.h>
#include <cmath>
#define SWITCH_TO_V "changeAttentionToV"
#define SWITCH_TO_T "changeAttentionToT"

using namespace yarp::os;
using namespace std;
class AttentionModule: public RFModule {
private:
	string robotName;
	Port sensorialContextInputPort;
	Port mentalActionsInputPort;
	Port attendedContextOutputPort;
	BufferedPort<Bottle> emotionalContextInputPort;
	Json::Reader jsonReader;
	Json::FastWriter jsonWriter;
	Json::Value attendedContext;
	Json::Value emotionalContext;
	float emotionalAttatchmentWeight;
	float movementSaliency;
	float brightnessSaliency;
	bool changeAttentionV,changeAttentionT;
	//bool engagedObjIsPerceived;
	Json::Value visualEngagement,tactilEngagement;
	Json::Value colors;
	bool lostObject;
	int cycles;

public:
	AttentionModule() {
		jsonReader.parse("[]", attendedContext);
		jsonReader.parse("[]", emotionalContext);
		jsonReader.parse("{}", tactilEngagement);
		jsonReader.parse("{}", visualEngagement);
		movementSaliency = 0.0;
		brightnessSaliency = 0.0;
		emotionalAttatchmentWeight=0.0;
		changeAttentionV = false;
		changeAttentionT = false;
		lostObject = true;
		cycles = 0;
	}

	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		yDebug(" Attention: robot name: %s\n", robotName.c_str());

		movementSaliency =
				rf.check("movement_saliency", Value("0.3")).asFloat32();
		brightnessSaliency =
				rf.check("brightness_saliency", Value("0.2")).asFloat32();
		emotionalAttatchmentWeight =
				rf.check("emotional_attachment_weight", Value("0.4")).asFloat32();

		if (!sensorialContextInputPort.open(
				"/" + robotName + "/attention/sensorialContext:i")) {
			yError("Failed creating input port for sensorial context");
			return false;
		}

		if (!mentalActionsInputPort.open(
				"/" + robotName + "/attention/mentalActions:i")) {

			yError("Failed creating input port for attention switch");
			return false;
		}

		if (!attendedContextOutputPort.open(
				"/" + robotName + "/attention/attendedContext:o")) {
			yError("Failed creating output port for attended context");
			return false;
		}

		if (!emotionalContextInputPort.open(
				"/" + robotName + "/attention/emotionalContext:i")) {
			yDebug(" Attention: Failed creating input port for emotional context");
			return false;
		}

		loadColors(rf.check("perceptual_memory", Value(".")).asString());
		return true;
	}

	virtual bool close() {
		sensorialContextInputPort.close();
		mentalActionsInputPort.close();
		attendedContextOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		yDebug(" Attention: cycle %d",++cycles);
		Bottle input;
		Bottle *buffInput;
		string input_string;

		//buffInput->clear();
		buffInput = emotionalContextInputPort.read(false); // non blocking read
		if (buffInput != NULL) {
			//yDebug(" Attention: Got: %s",buffInput->toString().c_str());
			input_string = buffInput->toString();
			prepareInput(input_string);
			jsonReader.parse(input_string.c_str(), emotionalContext);
			//yDebug("Previous emotional context%s\n",emotionalContext.toStyledString().c_str());
		} else {
			//yDebug(" Attention: There is no previous emotional context\n");
		}

		yDebug(" Attention: waiting for sensorial context...\n");
		input.clear();
		sensorialContextInputPort.read(input);
		Json::Value sensorialContext;
		//yDebug(" Attention: read %s\n",input.toString().c_str());
		input_string = input.toString();
		prepareInput(input_string);
		//yDebug(" Attention: read cleaned%s\n",input.toString().c_str());
		jsonReader.parse(input_string.c_str(), sensorialContext);
		yDebug(" Attention: visually engaged to: %s",visualEngagement.toStyledString().c_str());
		lostObject = true;
		if (sensorialContext[0].size() != 0) {

			yDebug(" Attention: Got sensorial context: %s",sensorialContext.toStyledString().c_str());
			float salencyBias = 0.0;
			float saliency = 0.0;
			Json::Value secondBest;
			jsonReader.parse("[]", attendedContext);

			for (Json::Value obj : sensorialContext[0]) {
				if(visualEngagement.size()!=0 && utils::areAboutSameObject(obj,visualEngagement)){
					yDebug("found engaged to");
					lostObject = false;
				}
				float brightness = getColorBrightness(obj["color"].asString());
				//yDebug(" Attention: Brightness for %s = %f",obj["color"].asString().c_str(),brightness);
				saliency = (
						(obj.isMember("moving") && obj["moving"].asBool()) ?
								1 : 0) * movementSaliency
						+ ((brightness/100)*brightnessSaliency);
				if(emotionalContext[0].size()>0)
					saliency+=(utils::areAboutSameObject(obj,visualEngagement)?1:0)*emotionalAttatchmentWeight;
				//yDebug("obj %s saliency=%f saliencyBias =%f",obj.toStyledString().c_str(),saliency,salencyBias);
				if (saliency > salencyBias) {
					//yDebug("saliency > salencyBias");
					salencyBias = saliency;
					//yDebug(" Attention: Before changin\n"<<attendedContext[0].toStyledString();

					secondBest = attendedContext[0];

					attendedContext[0] = obj;
					//yDebug(" Attention: After changin\n"<<attendedContext[0].toStyledString();
				}else if(secondBest.size()==0){
					secondBest = obj;
				}

				//yDebug(" Attention: Secondbest=\n"<<secondBest.toStyledString();
			}

			if(changeAttentionV){
				yDebug(" Attention: Changing attention to second best\n");
				attendedContext[0] = secondBest;
				changeAttentionV = false;
			}

		} else {
			attendedContext[0] = sensorialContext[0];
			//attendedContext[0]["engagedTo"] = visualEngagement;
		}

		yDebug(" Attention: lost object = %d",lostObject);
		attendedContext[1] = sensorialContext[1];
		yDebug(" Attention: Attended context: %s \n",attendedContext.toStyledString().c_str());

		Bottle output;// = attendedContextOutputPort.prepare();
		Json::Value attendedOutput = attendedContext;

		if(attendedOutput[0].size()!=0 && visualEngagement.size()!=0 && lostObject){
			attendedOutput[0]["lostObject"] = true;
		}

		output.addString(jsonWriter.write(attendedOutput));
		//yDebug(" Attention: Writing out attended context -- %s",output.toString().c_str());
		attendedContextOutputPort.write(output);


		yDebug(" Attention: waiting for mental actions...\n");
		//input;
		mentalActionsInputPort.read(input);
		//if (input!=NULL) {
		//contextInputPort.read(input);
		input_string = input.toString();
		prepareInput(input_string);
		Json::Value commandedActions;
		jsonReader.parse(input_string.c_str(), commandedActions);
		yDebug(" Attention: got mental actions %s\n",commandedActions.toStyledString().c_str());
		if(!lostObject){
		visualEngagement = attendedContext[0];
		}
		for (Json::Value& action : commandedActions) {
			yDebug(" Attention: ACTION %s\n",action.toStyledString().c_str());
		if( action=="showInterestInV"){
			visualEngagement = attendedContext[0];
			yDebug(" Attention: Engaged to %s",visualEngagement.toStyledString().c_str());
		}else if(action=="showInterestInT"){
			tactilEngagement = attendedContext[1];
			yDebug(" Attention: Engaged to %s",tactilEngagement.toStyledString().c_str());
		}else if(action=="changeAttentionV"){
			changeAttentionV = true;
		}else if(action=="changeAttentionT"){
			changeAttentionT = true;
		}
		}

		return true;
	}

private:

	bool loadColors(string colorsPath) {
		yDebug(" Attention: Loading %s\n", colorsPath.c_str());
		std::ifstream ifs;
		ifs.open(colorsPath);
		if (!ifs.is_open()) {
			std::cerr << "Could not load perceptual memory " << colorsPath
					<< "\n";
			return EXIT_FAILURE;
		}
		jsonReader.parse(ifs, colors);
		yDebug(" Attention: %s \n",colors.toStyledString().c_str());

		yDebug(" Attention:  Perceptual memory loaded [ok]\n");
		return true;
	}

	float getColorBrightness(string colorName) {
		//yDebug(" Attention:  Calculating brightness for %s \n",colorName.c_str());
		float brightness = 0.0;
		for (Json::Value color : colors["colors"]) {
			if (color["name"].asString() == colorName) {
				int R = floor(
						(double) (color["to"][0].asInt()
								+ color["from"][0].asInt()) / 2);
				int G = floor(
						(double) (color["to"][1].asInt()
								+ color["from"][1].asInt()) / 2);
				int B = floor(
						(double) (color["to"][2].asInt()
								+ color["from"][2].asInt()) / 2);
				brightness = 0.2126 * R + 0.7152 * G + 0.0722 * B;
				break;
			}
		}
		return brightness;

	}
};

