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
	BufferedPort<Bottle> attentionSwitchInputPort;
	Port attendedContextOutputPort;
	BufferedPort<Bottle> emotionalContextInputPort;
	Json::Reader jsonReader;
	Json::FastWriter jsonWriter;
	Json::Value attendedContext;
	Json::Value emotionalContext;
	float emotionalAttatchmentWeight;
	float movementSaliency;
	float brightnessSaliency;

	Json::Value colors;

public:
	AttentionModule() {
		jsonReader.parse("[]", attendedContext);
		jsonReader.parse("[]", emotionalContext);
		movementSaliency = 0.0;
		brightnessSaliency = 0.0;
		emotionalAttatchmentWeight=0.0;
	}

	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		printf("robot name: %s\n", robotName.c_str());

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

		if (!attentionSwitchInputPort.open(
				"/" + robotName + "/attention/attentionSwitch:i")) {

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
			printf("Failed creating input port for emotional context");
			return false;
		}

		loadColors(rf.check("perceptual_memory", Value(".")).asString());

		return true;
	}

	virtual bool close() {
		sensorialContextInputPort.close();
		attentionSwitchInputPort.close();
		attendedContextOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		Bottle input;
		Bottle *buffInput;
		string input_string;
		std::cout << "getting attention switch...\n";
		//attentionSwitchInputPort.read(false)
		buffInput = attentionSwitchInputPort.read(false); // non blocking read
		if (buffInput != NULL) {
			std::cout << "Got: " << buffInput->toString() << '\n';
		} else {
			std::cout << "attention switch not set \n";
		}
		//buffInput->clear();
		buffInput = emotionalContextInputPort.read(false); // non blocking read
		if (buffInput != NULL) {
			std::cout << "Got: " << buffInput->toString() << '\n';
			input_string = buffInput->toString();
			prepareInput(input_string);
			jsonReader.parse(input_string.c_str(), emotionalContext);
			yDebug("Previous emotional context%s\n",emotionalContext.toStyledString().c_str());
		} else {
			std::cout << "There is no previous emotional context\n";
		}

		std::cout << "waiting for sensorial context...\n";
		input.clear();
		sensorialContextInputPort.read(input);
		Json::Value sensorialContext;
		yDebug("read %s\n",input.toString().c_str());
		input_string = input.toString();
		prepareInput(input_string);
		yDebug("read cleaned%s\n",input.toString().c_str());
		jsonReader.parse(input_string.c_str(), sensorialContext);
		if (sensorialContext[0].size() != 0) {

			std::cout << "Got sensorial context: " << '\n'
					<< sensorialContext.toStyledString() << '\n';
			float salencyBias = 0.0;
			float saliency = 0.0;

			for (Json::Value obj : sensorialContext[0]) {
				float brightness = getColorBrightness(obj["color"].asString());
				std::cout << "Brightness: " << brightness << '\n';
				saliency = (
						(obj.isMember("moving") && obj["moving"].asBool()) ?
								1 : 0) * movementSaliency
						+ (brightness * brightnessSaliency);
				if(emotionalContext[0].size()>0)
				salencyBias+=(utils::areAboutSameObject(obj,emotionalContext[0])?1:0)*emotionalAttatchmentWeight;

				if (saliency > salencyBias) {
					salencyBias = saliency;
					attendedContext[0] = obj;
				}
			}
		} else {
			attendedContext[0] = sensorialContext[0];
		}

		attendedContext[1] = sensorialContext[1];
		std::cout << "Attended context: " << '\n'
				<< attendedContext.toStyledString() << '\n';

		Bottle output;
		output.addString(jsonWriter.write(attendedContext));
		yDebug(" Writing out attended context");
		attendedContextOutputPort.write(output);
		return true;
	}

private:

	bool loadColors(string colorsPath) {
		printf("Loading %s\n", colorsPath.c_str());
		std::ifstream ifs;
		ifs.open(colorsPath);
		if (!ifs.is_open()) {
			std::cerr << "Could not load perceptual memory " << colorsPath
					<< "\n";
			return EXIT_FAILURE;
		}
		jsonReader.parse(ifs, colors);
		std::cout << colors.toStyledString() << '\n';

		std::cout << " Perceptual memory loaded [ok]" << '\n';
		return true;
	}

	float getColorBrightness(string colorName) {
		std::cout << " Calculating brightness for " << colorName << '\n';
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

