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
#include <jutils.h>

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
	BufferedPort<ImageOf<PixelBgr>> rawImageMotionCUTOutputPort;
	BufferedPort<ImageOf<PixelBgr> >blobbedImageOutputPort;
	BufferedPort<ImageOf<PixelBgr> > processedImageOutputPort;
	BufferedPort<Bottle> movingBlobsInputPort;
	Port sensorialContextOutputPort;
	Port continueInputPort;
	Json::Value perceptualMemory;

	Json::Reader jsonReader;
	Json::FastWriter fastWriter;

public:
	virtual bool configure(ResourceFinder &rf) {

		printf("Configuring...");
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

		if (!blobbedImageOutputPort.open(
								"/" + robotName + "/perception/blobImage:o")) {

							yError("Failed creating output port for blob image");
							return false;
						}
		if (!movingBlobsInputPort.open(
										"/" + robotName + "/perception/movingBlobs:i")) {

									yError("Failed creating input port for moving blobs");
									return false;
								}

		//movingBlobsInputPort.setTimeout(0.5);
		if (!processedImageOutputPort.open(
						"/" + robotName + "/perception/processedImage:o")) {

					yError("Failed creating output port for processed image");
					return false;
				}


		if (!sensorialContextOutputPort.open(
				"/" + robotName + "/perception/sensorialContext:o")) {
			yError("Failed creating output port for sensorial context");

		}

		if (!continueInputPort.open(
						"/" + robotName + "/perception/continue:i")) {
					yError("Failed creating input port for continue command");

				}

		loadPerceptualMemory(rf.check("perceptual_memory", Value(".")).asString());
		return true;


		//TODO: load sensorial memory
	}

	virtual bool close() {
		rawImageInputPort.close();
		processedImageOutputPort.close();
		sensorialContextOutputPort.close();
		continueInputPort.close();
		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {

		std::cout << "waiting for an image..." << std::endl;

				ImageOf<PixelBgr> *inputRawImage = rawImageInputPort.read();
				IplImage *tmp_ipl_image = (IplImage*) inputRawImage->getIplImage();
				IplImage *cloned_image = cvCloneImage(tmp_ipl_image);
				Mat imgMat = cvarrToMat(tmp_ipl_image);
				Mat im_with_keypoints = imgMat;
				//IplImage *blobImg;
				//blobImg = cvCreateImageHeader(cvSize(tmp_ipl_image->width,tmp_ipl_image->height),8,3);
				//cvZero(blobImg);

				Mat blob_keypoints;
				printf("TYPE => %d",imgMat.type());
				cvZero(cloned_image);
				// imgMat.copyTo(blob_keypoints);
				blob_keypoints = cvarrToMat(cloned_image);

				 //blob_keypoints = cv::Mat::zeros(tmp_ipl_image->height,tmp_ipl_image->width,CV_8UC3);
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
							perceivedObj["x"] = i->pt.x;
							perceivedObj["y"] = i->pt.y;
							sensorialContext[0][objId] = perceivedObj;
						}
						drawKeypoints(im_with_keypoints, keypoints, im_with_keypoints,
								Scalar(0, 255, 0),
								DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

						drawKeypoints(blob_keypoints, keypoints, blob_keypoints,
														Scalar(0, 255, 0),
														DrawMatchesFlags::DEFAULT);


					}

				}

				ImageOf<PixelBgr> &outputBlobImage = blobbedImageOutputPort.prepare();

				IplImage blobbedImage = IplImage(blob_keypoints);
				outputBlobImage.wrapIplImage(&blobbedImage);

				std::cout << "Writing out blobbed image " << std::endl;

				blobbedImageOutputPort.write();

				std::cout << "Waiting for moving blobs " << std::endl;


				//Bottle list_of_moving_blobs;
				//movingBlobsInputPort.setTimeout(1.0);
				Bottle *list_of_moving_blobs = movingBlobsInputPort.read();

				std::cout << "Got blobs: " << list_of_moving_blobs->toString().c_str()<<"\n"<<std::endl;
				for(int i=0;i<list_of_moving_blobs->size();i++){
					Bottle * blob = list_of_moving_blobs->get(i).asList();
					Point a(blob->get(0).asFloat32(), blob->get(1).asFloat32());
					float minDist = std::numeric_limits<float>::max();
					string closestblob = "";
					std::cout <<" blob " << i << ": x="<<blob->get(0).asInt()<< ", y="<<blob->get(1).asInt()<<"\n"<<std::endl;
					for(Json::Value key:sensorialContext[0].getMemberNames()){
						Point b(sensorialContext[0][key.asString()]["x"].asFloat(), sensorialContext[0][key.asString()]["y"].asFloat());
						//mb.
						float dist = cv::norm(a-b);
						std::cout << "Euclidian distance: " << cv::norm(a-b)<<"\n"<<std::endl;
						if(dist<=minDist){
							minDist = dist;
							closestblob = key.asString();
						}
					}

					if(closestblob!=""){
						sensorialContext[0][closestblob]["moving"] = true;
					}
				}

				for(Json::Value key:sensorialContext[0].getMemberNames()){
					sensorialContext[0][key.asString()].removeMember("x");
					sensorialContext[0][key.asString()].removeMember("y");
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


				ImageOf<PixelBgr> &outputProcessedImage = processedImageOutputPort.prepare();


				outputProcessedImage.resize(inputRawImage->width(), inputRawImage->height());
				outputProcessedImage.wrapIplImage(&blobedImage);


		        //ImageOf<PixelBgr> &outputProcessedImage = processedImageOutputPort.prepare(); //get an output image


		        //outputProcessedImage.resize(inputRawImage->width(), inputRawImage->height());

				//outputImage = yarpReturnImage;




				std::cout << "Writing out processed image " << std::endl;

				processedImageOutputPort.write();

				std::cout << "Sensorial  context: "
						<< sensorialContext.toStyledString() << std::endl;

				std::cout << "writing out sensorial  context\n";
Bottle output;
output.addString(fastWriter.write(sensorialContext));
				sensorialContextOutputPort.write(output);
				std::cout << "Waiting for continue signal\n";
Bottle input;
				//continueInputPort.read(input);
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

