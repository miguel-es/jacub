// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/**
* @ingroup icub_tutorials
* \defgroup imageProc imageProc
*/

#include <stdio.h>
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
    BufferedPort<Bottle> contextOutPort;


    imagePort.open("/imageProc/image/in");  // give the port a name
    outPort.open("/imageProc/image/out");
    contextOutPort.open("/perception/context/out");


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
        Bottle &outContext = contextOutPort.prepare(); //get an output image
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


 //printf("\nblobs with features: color=%s and size=%s -> %d\n",color.name.c_str(),area.name.c_str(),keypoints.size());

for (std::vector<KeyPoint>::const_iterator i = keypoints.begin(); i != keypoints.end(); ++i){
//nblobs++;
//printf("num blob %d\n",nblobs);
    //std::cout << i->pt << ' ';
           int xpos = floor(i->pt.x / 106);
            int ypos = floor(i->pt.y / 80);
            rapidjson::Value object(rapidjson::kObjectType);

            rapidjson::Value colorv;
		colorv.SetString(color.name.c_str(), allocator);

		rapidjson::Value sizev;
		sizev.SetString(area.name.c_str(), allocator);

    object.AddMember("color", colorv, allocator);
    object.AddMember("size", sizev, allocator);
    object.AddMember("attended", true, allocator);
    //string blobname = "obj"+to_string(nblobs);
    //printf("nam3 => %s",blobname.c_str());

    object.AddMember("attended", sizev, allocator);
    rapidjson::Value objname;
		objname.SetString(("obj"+to_string(++nblobs)).c_str(), allocator);

    object.AddMember("sector", 3*ypos+xpos+1,allocator);
    document.AddMember(objname, object, allocator);

}

 //printf("\n");
	drawKeypoints( im_with_keypoints, keypoints, im_with_keypoints, Scalar(0,255,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

       // printf("Context %s\n", str);

        }

        }

        StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	document.Accept(writer);

	std::cout << "Visual context: "<< strbuf.GetString() << std::endl;

	//attend(document);
     /*   StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        context.Accept(writer);
        auto str = sb.GetString();*/
       // printf("Context %s\n", str);
	//IplImage gray_img_hdr;

	  line(im_with_keypoints, Point(0,80), Point(image->width(),80), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(0,160), Point(image->width(),160), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(106,0), Point(106,image->height()), Scalar(0, 255, 0),1,8,0);
	  line(im_with_keypoints, Point(212,0), Point(212,image->height()), Scalar(0, 255, 0),1,8,0);



	IplImage blobed = IplImage( im_with_keypoints );
	//IplImage* blobed = (IplImage*)(IplImage(&im_with_keypoints));

	//IplImage *blobed = cvGetImage(&im_with_keypoints, &gray_img_hdr);
//addCircle(*image,PixelRgb(255,0,0),image->width()/2,image->height()/2,image->height()/4);

//circle_t c;
  //      detectAndDraw(imgMat, 1.3, c);

//IplImage* p_ipl_image_in = cvCreateImage( cvSize(image->width,  image->height),
									//image->depth, 3 );

//Mat in_cv = yarp::cv::toCvMat(outImage);
        //outImage = img;

    //cvNamedWindow("test",1);
    //cvShowImage("test",cvImage);


//		VisualContext v;
//v.size="fuck";
//if(outContext.size=="holis"){printf("igual\n");}
//printf(v.size.c_str());
 //printf("size igual a %s\n",outContext.size.c_str());
 //printf("size igual a holis nop %s \n",outContext.size.c_str());

 //if(outContext.content().size=="holis"){printf("igual");}else{printf("diferet2");}
  //  printf("color %i, %s",outContext.content().color,outContext.content().size);


  ImageOf<PixelBgr> yarpReturnImage;
    yarpReturnImage.wrapIplImage(&blobed);
        outImage=yarpReturnImage;

       /* if (image!=NULL) { // check we actually got something
           // printf("We got an image of size %dx%d\n", image->width(), image->height());
            double xMean = 0;
            double yMean = 0;
            double xBoxMax = image->width()-1;
            double yBoxMax = image->height()-1;
            double xBoxMin = -1;
            double yBoxMin = -1;

            int ct = 0;
            for (int x=0; x<outImage->width(); x++) {
                for (int y=0; y<outImage.height(); y++) {
                    PixelRgb& pixel = image.pixel(x,y);
                    // very simple test for blueishness
                    // make sure blue level exceeds red and green by a factor of 2
                    if (pixel.b>pixel.r*1.2+10 && pixel.b>pixel.g*1.2+10) {
                        // there's a blueish pixel at (x,y)!
                        // let's find the average location of these pixels
                        if(x<xBoxMax) xBoxMax = x;
                        if(y<yBoxMax) yBoxMax = y;
                        if(x>xBoxMin) xBoxMin = x;
                        if(y>yBoxMin) yBoxMin = y;

                        xMean += x;
                        yMean += y;
                        ct++;

                        outImage(x,y).r=pixel.r;
                        outImage(x,y).g=pixel.g;
                        outImage(x,y).b=pixel.b;
                    }else{
                        double gray = (outImage(x,y).r+outImage(x,y).g+outImage(x,y).b)/3;
                        outImage(x,y).r=gray;
                        outImage(x,y).g=gray;
                        outImage(x,y).b=gray;
                    }
                    if(x==106 or x==212){
                        outImage(x,y).r = 0;
                        outImage(x,y).b = 0;
                        outImage(x,y).g = 255;
                    }
                    if(y==80 or y==160){
                        outImage(x,y).r = 0;
                        outImage(x,y).b = 0;
                        outImage(x,y).g = 255;

                    }
                }
            }
            //printf("Bounding box xmin %g ymin %g xmax %g ymax %g",xBoxMin,yBoxMin,xBoxMax,yBoxMax);
            double centroidx = xBoxMax +floor((xBoxMin-xBoxMax)/2);
            double centroidy = yBoxMax+floor((yBoxMin-yBoxMax)/2);
            //printf("Centroid (%g, %g)",centroidx,centroidy);
            outImage(centroidx,centroidy).r = 255;
            outImage(centroidx,centroidy).b = 0;
            outImage(centroidx,centroidy).g = 0;
            outImage(centroidx+1,centroidy).r = 255;
            outImage(centroidx+1,centroidy).b = 0;
            outImage(centroidx+1,centroidy).g = 0;
            outImage(centroidx-1,centroidy).r = 255;
            outImage(centroidx-1,centroidy).b = 0;
            outImage(centroidx-1,centroidy).g = 0;

            outImage(centroidx,centroidy+1).r = 255;
            outImage(centroidx,centroidy+1).b = 0;
            outImage(centroidx,centroidy+1).g = 0;
            outImage(centroidx,centroidy-1).r = 255;
            outImage(centroidx,centroidy-1).b = 0;
            outImage(centroidx,centroidy-1).g = 0;

            if (ct>0) {
                xMean /= ct;
                yMean /= ct;
            }
            if (ct>(image->width()/20)*(image->height()/20)) {
               // printf("Best guess at blue target: %g %g\n", xMean, yMean);
            }

            outPort.write();*/
            outContext.addString(strbuf.GetString());

            contextOutPort.write();
            /*
            int sector = 0;
            int xpos = floor(centroidx / 106);
            int ypos = floor(centroidy / 80);
            //printf("Centroid coordinates: %d, %d\n", xpos, ypos);
            Document context;
            context.SetObject();
            Document::AllocatorType& allocator = context.GetAllocator();
            context.AddMember("color", "blue", allocator);
            context.AddMember("size", "s1", allocator);
            context.AddMember("moving", false, allocator);
            context.AddMember("sector", 3*ypos+xpos+1,allocator);

            StringBuffer sb;
        PrettyWriter<StringBuffer> writer(sb);
        context.Accept(writer);
        auto str = sb.GetString();
       // printf("Context %s\n", str);


            //printf(string(context.GetString()));

        }*/
        outPort.write();
    }
    return 0;
}



