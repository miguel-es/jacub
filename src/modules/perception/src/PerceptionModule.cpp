// Author: Miguel Estrada - <luism@unam.mx>

#include <stdio.h>
#include <iostream>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/BinPortable.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Property.h>
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>

#include <string>
#include <cmath>

#include <fstream>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

#include <yarp/sig/all.h>
#include <jutils.cpp>

using namespace std;
using namespace cv;
using namespace yarp::sig;
using namespace yarp::os;
using namespace yarp::sig::draw;
using namespace yarp::sig::file;

class PerceptionModule: public RFModule {
private:
	string robotName;
	BufferedPort<ImageOf<PixelBgr> > rawImageInputPort;
	BufferedPort<ImageOf<PixelBgr> > processedImageOutputPort;
	Port sensorialContextOutputPort;
	Json::Value perceptualMemory;

	Json::Reader jsonReader;
	Json::FastWriter fastWriter;

public:
	virtual bool configure(ResourceFinder &rf) {

		robotName = rf.check("robot", Value("jacub")).asString();
		printf("robot name: %s\n", robotName.c_str());

		if (!rawImageInputPort.open(
				"/" + robotName + "/perception/rawImage:i")) {
			yError("Failed creating input port for raw image");
			return false;
		}

		if (!processedImageOutputPort.open(
				"/" + robotName + "/perception/processedImage:o")) {

			yError("Failed creating output port for processed image");
			return false;
		}

		if (!sensorialContextOutputPort.open(
				"/" + robotName + "/perception/sensorialContext:o")) {
			yError("Failed creating output port for sensorial context");

		}
		return true;
		//TODO: load sensorial memory
	}

	virtual bool close() {
		rawImageInputPort.close();
		processedImageOutputPort.close();
		sensorialContextOutputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {

		std::cout << "waiting for an image..." << std::endl;

				ImageOf<PixelBgr> *inputRawImage = rawImageInputPort.read();
				IplImage *tmp_ipl_image = (IplImage*) inputRawImage->getIplImage();
				Mat imgMat = cvarrToMat(tmp_ipl_image);
				Mat im_with_keypoints = imgMat;

				int nblobs = 0;
				Json::Value sensorialContext;

				jsonReader.parse("[{},{}]", sensorialContext); // initialization

				//First construct sensorial context for vision
				for (Json::Value color : perceptualMemory["colors"]) {
					for (Json::Value area : perceptualMemory["sizes"]) {

						printf("Trying to detect objects of color:%s and size:%s\n",
								color["name"].asString().c_str(),
								area["name"].asString().c_str());
		//

						/*printf("area => %s\n", area.toStyledString().c_str());*/
						SimpleBlobDetector::Params params;

						// Change thresholds
						params.minThreshold = 30;
						params.maxThreshold = 200;

						// Filter by Area.
						params.filterByArea = true;
						/*printf("area between %d - %d\n", area["from"].asInt(),
								area["to"].asInt());*/
						params.minArea = area["from"].asInt();
						params.maxArea = area["to"].asInt();

						// Filter by Circularity
						//params.filterByCircularity = true;
						//params.minCircularity = 0.1;

						// Filter by Convexity
							//	params.filterByConvexity = true;
						//params.minConvexity = 0.87;

						// Filter by Inertia
						//params.filterByInertia = true;
						//params.minInertiaRatio = 0.01;
						params.filterByColor = true;
						params.blobColor = 255;

						  Ptr<SimpleBlobDetector> detector = SimpleBlobDetector::create(params);
						//SimpleBlobDetector detector(params);

						Mat color_filtered_img;
/*
						std::cout << "Color from : " << color["from"][0].asInt() << ","
								<< color["from"][1].asInt() << ","
								<< color["from"][2].asInt() << "," << std::endl;
						std::cout << "Color to : " << color["to"][0].asInt() << ","
								<< color["to"][1].asInt() << ","
								<< color["to"][2].asInt() << "," << std::endl;*/

						cv::Scalar from = cv::Scalar(color["from"][0].asInt(),
								color["from"][1].asInt(), color["from"][2].asInt());
						cv::Scalar to = cv::Scalar(color["to"][0].asInt(),
								color["to"][1].asInt(), color["to"][2].asInt());

						inRange(imgMat, from, to, color_filtered_img);

						std::vector<KeyPoint> keypoints;
						detector->detect(color_filtered_img, keypoints);

						for (std::vector<KeyPoint>::const_iterator i =
								keypoints.begin(); i != keypoints.end(); ++i) {
							string objId = "obj" + to_string(++nblobs);
							//perceptualContext.append(objId.c_str());

							int xpos = floor(i->pt.x / 106);
							int ypos = floor(i->pt.y / 80);
							//rapidjson::Value object(rapidjson::kObjectType);

							Json::Value perceivedObj;
							//perceivedObj.append("color");

							perceivedObj["color"] = color["name"].asString();

							//Json::Value perceivedSize;
							//perceivedObj.append("size");
							perceivedObj["size"] = area["name"].asString();
							//sizev.SetString(area.name.c_str(), allocator);

							//object.AddMember("color", colorv, allocator);
							//object.AddMember("size", sizev, allocator);

							//Json::Value objName;
							//perceivedObj.SetString(("obj"+to_string(++nblobs)).c_str(), allocator);

							//perceivedObj.append("sector");
							perceivedObj["sector"] = 3 * ypos + xpos + 1;
							sensorialContext[0][objId] = perceivedObj;
						}
						drawKeypoints(im_with_keypoints, keypoints, im_with_keypoints,
								Scalar(0, 255, 0),
								DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

					}

				}

				//Draw sector divisions over over the image with the objects identified

				line(im_with_keypoints, Point(0, 80), Point(inputRawImage->width(), 80),
						Scalar(0, 255, 0), 1, 8, 0);
				line(im_with_keypoints, Point(0, 160), Point(inputRawImage->width(), 160),
						Scalar(0, 255, 0), 1, 8, 0);
				line(im_with_keypoints, Point(106, 0), Point(106, inputRawImage->height()),
						Scalar(0, 255, 0), 1, 8, 0);
				line(im_with_keypoints, Point(212, 0), Point(212, inputRawImage->height()),
						Scalar(0, 255, 0), 1, 8, 0);



				IplImage blobedImage = IplImage(im_with_keypoints);
				ImageOf<PixelBgr> outputProcessedImage = processedImageOutputPort.prepare();
				outputProcessedImage.resize(inputRawImage->width(), inputRawImage->height());
				outputProcessedImage.wrapIplImage(&blobedImage);

		        //ImageOf<PixelBgr> &outputProcessedImage = processedImageOutputPort.prepare(); //get an output image


		        //outputProcessedImage.resize(inputRawImage->width(), inputRawImage->height());

				//outputImage = yarpReturnImage;

				processedImageOutputPort.write();

				std::cout << "Sensorial  context: "
						<< sensorialContext.toStyledString() << std::endl;

				std::cout << "writing out sensorial  context\n";
Bottle output;
output.addString(fastWriter.write(sensorialContext));
				sensorialContextOutputPort.write(output);
		return true;
	}

private:

	bool loadPerceptualMemory(string perceptualMemoryPath) {
		printf("Loading %s\n", perceptualMemoryPath.c_str());
		std::ifstream ifs;
		ifs.open(perceptualMemoryPath);
		if (!ifs.is_open()) {
			std::cerr << "Could not load perceptual memory "
					<< perceptualMemoryPath << "\n";
			return EXIT_FAILURE;
		}
		jsonReader.parse(ifs, perceptualMemory);
		std::cout << perceptualMemory.toStyledString() << '\n';

		std::cout << " Perceptual memory loaded [ok]" << '\n';
		return true;
	}
};

int main(int argc, char *argv[]) {

	Network yarp;

	if (!yarp.checkNetwork()) {
		yError("Yarp server does not seem available\n");
		return 1;
	}

	PerceptionModule perception;

	ResourceFinder rf;
	rf.setVerbose(); //logs searched directories
	rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
	rf.configure(argc, argv);

	string robotName = rf.check("robot", Value("jacub")).asString();

	perception.runModule(rf);
}

