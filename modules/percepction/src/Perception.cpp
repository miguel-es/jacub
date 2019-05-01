// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
* @ingroup icub_tutorials
* \defgroup imageProc imageProc
*/

#include <stdio.h>
#include <iostream>
#include <yarp/os/Network.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/BinPortable.h>
#include <yarp/sig/Image.h>
#include <yarp/os/Time.h>
#include <yarp/os/Property.h>

#include <string>
#include <cmath>
#include "VisualContext.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"


#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

#include <yarp/sig/all.h>

 using namespace std;
using namespace cv;
using namespace yarp::sig;
using namespace yarp::os;
using namespace rapidjson;
using namespace yarp::sig::draw;
using namespace yarp::sig::file;

struct color_range {
  cv::Scalar from;
  cv::Scalar to;
  string name;
};

struct area_range {
  int from;
  int to;
  string name;
};


int main(int argc, char *argv[])
{
    Network yarp;
    VisualContext v;
    BufferedPort<ImageOf<PixelBgr> > imagePort;  // make a port for reading images
    BufferedPort<ImageOf<PixelBgr> > outPort;
    BufferedPort<Bottle> output_port;

    Port input_port;
    Port dever_port;


    imagePort.open("/imageProc/image/in");  // give the port a name
    outPort.open("/imageProc/image/out");
    output_port.open("/jacub/perception/context/out");
    input_port.open("/jacub/perception/emotion/in");
    dever_port.open("/jacub/perception/iDevER");

    color_range blue;
    blue.from = cv::Scalar(100, 0, 0);
    blue.to = cv::Scalar(255, 0, 0);
    blue.name = "blue";
    color_range red;
    red.from = cv::Scalar(0, 0, 100);
    red.to = cv::Scalar(0, 0, 255);
    red.name = "red";

    //learned colors
        color_range colors[] = {blue,red};


    area_range s1;
    s1.from = 20;
    s1.to = 100;
    s1.name = "s1";
    area_range s2;
    s2.from = 101;
    s2.to = 10000;
    s2.name = "s2";
    //learned colors
    area_range sizes[] = {s1, s2};



    while (true) { // repeat forever
        ImageOf<PixelBgr> *image = imagePort.read();  // read an image
        ImageOf<PixelBgr> &outImage = outPort.prepare(); //get an output image
        Bottle &outContext = output_port.prepare(); //get an output image
        outContext.clear();
        //outContext.color="c1";
//outContext.size="s1";
//outContext.moving=true;

outImage.resize(image->width(), image->height());


IplImage *tmp_ipl_image = (IplImage*) image->getIplImage();
Mat imgMat = cvarrToMat(tmp_ipl_image);
Mat im_with_keypoints = imgMat;

int nblobs = 0;
rapidjson::Document document;

	// define the document as an object rather than an array
	document.SetObject();

	// create a rapidjson array type with similar syntax to std::vector
	//rapidjson::Value objs(rapidjson::kArrayType);

	// must pass an allocator when the object may need to allocate memory
	rapidjson::Document::AllocatorType& allocator = document.GetAllocator();

//printf("Detecting blobs\n");
bool attending = false;
for (color_range color : colors) {
for (area_range area : sizes) {

        //printf("Detecting objects of color:%s and size:%s\n",color.name.c_str(),area.name.c_str());
//

    SimpleBlobDetector::Params params;

	// Change thresholds
	params.minThreshold = 30;
	params.maxThreshold = 200;

	// Filter by Area.
	params.filterByArea = true;
	//printf("area between %d - %d" ,area.from,area.to);
	params.minArea = area.from;
	params.maxArea = area.to;

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

	    SimpleBlobDetector detector(params);

Mat color_filtered_img;

	  inRange(imgMat, color.from, color.to, color_filtered_img);

std::vector<KeyPoint> keypoints;
detector.detect(color_filtered_img, keypoints);

for (std::vector<KeyPoint>::const_iterator i = keypoints.begin(); i != keypoints.end(); ++i){


           int xpos = floor(i->pt.x / 106);
            int ypos = floor(i->pt.y / 80);
            rapidjson::Value object(rapidjson::kObjectType);

            rapidjson::Value colorv;
		colorv.SetString(color.name.c_str(), allocator);

		rapidjson::Value sizev;
		sizev.SetString(area.name.c_str(), allocator);

    object.AddMember("color", colorv, allocator);
    object.AddMember("size", sizev, allocator);


    rapidjson::Value objname;
		objname.SetString(("obj"+to_string(++nblobs)).c_str(), allocator);

    object.AddMember("sector", 3*ypos+xpos+1,allocator);
    document.AddMember(objname, object, allocator);

}
	drawKeypoints( im_with_keypoints, keypoints, im_with_keypoints, Scalar(0,255,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

        }

        }
    string id_attendedobj ="";
   	if(nblobs>0){
    int att = (rand() % nblobs)+1;
    printf("attended => %d\n",att);
    id_attendedobj= "obj"+std::to_string(att);
    document[id_attendedobj.c_str()].AddMember("attended",true,allocator);
    }

        StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	document.Accept(writer);

	std::cout << "Visual context: "<< strbuf.GetString() << std::endl;

            line(im_with_keypoints, Point(0,80), Point(image->width(),80), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(0,160), Point(image->width(),160), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(106,0), Point(106,image->height()), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(212,0), Point(212,image->height()), Scalar(0, 255, 0),1,8,0);



	IplImage blobed = IplImage( im_with_keypoints );
  ImageOf<PixelBgr> yarpReturnImage;
    yarpReturnImage.wrapIplImage(&blobed);
        outImage=yarpReturnImage;

        outPort.write();


if(id_attendedobj!=""){
rapidjson::Value& attendedobj = document[id_attendedobj.c_str()];

	StringBuffer strbuf2;
	Writer<StringBuffer> writer2(strbuf2);
	attendedobj.Accept(writer2);

	std::cout << "Attended context: "<< strbuf2.GetString() << std::endl;

            outContext.addString(strbuf2.GetString());

            output_port.write();

	printf("Perception: waiting for emotion \n");
        Bottle input;
        input_port.read(input);
        //if (input!=NULL) {
            printf("Got %s\n",input.toString().c_str());
            rapidjson::Value emov;
		emov.SetString("+1", allocator);
		    rapidjson::Value emo;
		emo.SetString(input.toString().c_str(), allocator);

          document[id_attendedobj.c_str()].AddMember(emo,emov,allocator);

StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	document.Accept(writer);

	std::cout << "Visual context after emotion generation: "<< strbuf.GetString() << std::endl;


	printf("PM: Sending context to iDevER\n");
	Bottle att_context;
	            att_context.addString(strbuf2.GetString());
    dever_port.write(att_context);

    printf("PM: Waiting for iDevER to perform action\n");
    Bottle done;
    dever_port.read(done);
}



    }
    return 0;
}



