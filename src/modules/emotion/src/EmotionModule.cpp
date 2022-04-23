
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

using namespace yarp::os;
using namespace std;
class EmotionModule: public RFModule {
private:
	string robotName;
	RpcServer attendedContextInputPort;
	Port expectationsInputPort;
	Port emotionalContextOutputPort;
	Port facialExpressionOutputPort;
	Json::Reader jsonReader;
	Json::FastWriter jsonWriter;
	Json::Value emotionalContext;
	Json::Value expectations;
	int cycles;

public:
	EmotionModule() {
		jsonReader.parse("[]", emotionalContext);
		jsonReader.parse("{}", expectations);
		cycles = 0;
	}

	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		yDebug(" Emotion: robot name: %s\n", robotName.c_str());

		if (!attendedContextInputPort.open(
				"/" + robotName + "/emotion/attendedContext:i")) {
			yError("Failed creating input port for attended context");
			return false;
		}

		if (!expectationsInputPort.open(
				"/" + robotName + "/emotion/expectations:i")) {

			yError("Failed creating input port for attention switch");
			return false;
		}

		if (!emotionalContextOutputPort.open(
				"/" + robotName + "/emotion/emotionalContext:o")) {
			yError("Failed creating output port for emotional context");
			return false;
		}

		if (!facialExpressionOutputPort.open(
				"/" + robotName + "/emotion/facialExpression:o")) {
			yDebug(" Attention: Failed creating output port for facial expressions");
			return false;
		}

		return true;
	}

	virtual bool close() {
		attendedContextInputPort.close();
		expectationsInputPort.close();
		emotionalContextOutputPort.close();
		facialExpressionOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		yDebug(" Emotion: cycle %d",++cycles);
		Bottle input;
		string input_string;
		Json::Value attendedContext;
                yDebug(" Emotion: waiting for attended context...\n");
		input.clear();
		attendedContextInputPort.read(input,true);
		//yDebug(" Attention: read %s\n",input.toString().c_str());
		input_string = input.toString();
		prepareInput(input_string);
		jsonReader.parse(input_string.c_str(), attendedContext);
		emotionalContext = attendedContext
		//Se agregan las emociones al contexto
		//TODO: incluir los criterios establecidos para la generación de emociones
		//Por ejemplo si el objeto atendido está en el centro del campo de visión la emoción contentment tendrá valor 2
		//Si se perdió el objeto al cual estaba enganchado visualmente la emocion debería ser distress=1, etc...
		emotionalContext[0]["contentment"] = 1;
		emotionalContext[1]["contentment"] = 1;
		yDebug(" Emotion: attending to: %s",emotionalContext.toStyledString().c_str());

		Bottle output;// = emotionalContextOutputPort.prepare();
		yDebug(" Emotion: emotional context: %s \n",emotionalContext.toStyledString().c_str());
		output.addString(jsonWriter.write(emotionalContext));
		yDebug(" Emotion: Writing out attended context -- %s",output.toString().c_str());
		emotionalContextOutputPort.write(output);

		yDebug(" Emotion: waiting for expectations...\n");
		input.clear();
		expectationsInputPort.read(input,true);
		input_string = input.toString();
		prepareInput(input_string);
		yDebug(" Emotion: read expectations%s\n",input.toString().c_str());
		jsonReader.parse(input_string.c_str(), expectations);
		return true;
	}
};

