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
#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>


#include <string>
#include <cmath>
#include "VisualContext.h"

#include <fstream>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <opencv2/core/core.hpp>
#include <opencv2/opencv.hpp>

#include <yarp/sig/all.h>

#define CTRL_THREAD_PER     0.02    // [s]

using namespace std;
using namespace cv;
using namespace yarp::sig;
using namespace yarp::os;
using namespace yarp::sig::draw;
using namespace yarp::sig::file;

/*struct color_range
{
    cv::Scalar from;
    cv::Scalar to;
    string name;
};

struct area_range
{
    int from;
    int to;
    string name;
};*/

class PerceptionThread: public RateThread
{

string robotName;
    //VisualContext v;
    BufferedPort<ImageOf<PixelBgr> > inputImagePort;  // port to read images
    BufferedPort<ImageOf<PixelBgr> > outputImagePort;
    Port outputContextPort;

    Port inputEmotionPort;
    Port outputAttendedContextPort;
    Port propio_port;
    Json::Value perceptualMemory;

            Json::Reader jreader;

    //string perceptionJsonPath;
    // the event callback attached to the "motion-ongoing"

public:
    PerceptionThread(string robotName, const double period) : RateThread(int(period*1000.0))
    {
        // we wanna raise an event each time the arm is at 20%
        // of the trajectory (or 80% far from the target)
        //cartesianEventParameters.type="motion-ongoing";
        //cartesianEventParameters.motionOngoingCheckPoint=0.2;
        this->robotName = robotName;
        //this->perceptionJsonPath = perceptionJsonPath;

    }

    virtual bool threadInit()
    {

    string  local;
    string remote;
    local = "/"+robotName+"/perception/image:i";
    remote = "/icubSim/cam/left";

    //inputImagePort.open(local);

    if(!inputImagePort.open(local))
        {
            printf("Failed creating input port for receiving images");
            return false;
        }


        if(Network::connect(remote,local))
        {
            yInfo(" Stablished  port connection between %s and %s",remote.c_str(),local.c_str());
        }
        else
        {
            yWarning(" Failed stablishing connection between %s and %s. Is the robot available?\n",remote.c_str(),local.c_str());

        }

            local = "/"+robotName+"/perception/image:o";
            remote = "/leftview";

            outputImagePort.open("/"+robotName+"/perception/image:o");

            if(Network::connect(local,remote))
        {
            yInfo(" Stablished  port connection between %s and %s",local.c_str(),remote.c_str());
        }
        else
        {
            yWarning(" Failed stablishing connection between %s and %s. Is yarpview running?\n",local.c_str(),remote.c_str());

        }

    //


    local = "/"+robotName+"/perception/context:o";
            remote = "/"+robotName+"/emotion/context/in";

            outputContextPort.open(local);

            //outputImagePort.open("/"+robotName+"/perception/image:o");

            if(Network::connect(local,remote))
        {
            yInfo(" Stablished  port connection between %s and %s",local.c_str(),remote.c_str());
        }
        else
        {
            yWarning(" Failed stablishing connection between %s and %s. Is yarpview running?\n",local.c_str(),remote.c_str());

        }

    inputEmotionPort.open("/"+robotName+"/perception/emotion:i");
    outputAttendedContextPort.open("/"+robotName+"/perception/attention/context:o");

    //propio_port.open("/jacub/perception/propioception");

   /* std::ifstream ifs;
 ifs.open(perceptualMemoryPath);
        //std::ifstream ifs("("+kb_file+")" );
        if ( !ifs.is_open() )
        {
            std::cerr << "Could not load perception file "<<perceptualMemoryPath<<"\n";
            return EXIT_FAILURE;
        }

        Json::Reader reader;
        //Json::Value kb; // array of schemas
        reader.parse(ifs, perceptualMemory);*/

/*son::Reader reader;
        reader.parse("{\"schemes\":[]}",  kb);

    color_range c1;
    c1.from = cv::Scalar(100, 0, 0);
    c1.to = cv::Scalar(255, 0, 0);
    c1.name = "c1";
    color_range c2;
    c2.from = cv::Scalar(0, 0, 100);
    c2.to = cv::Scalar(0, 0, 255);
    c2.name = "c2";

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
    area_range sizes[] = {s1, s2};*/

    /*std::cout << "Perceptual memory: " << '\n' << perceptualMemory.toStyledString() << '\n';*/

        //Json::Reader reader;
        //reader.parse("{\"schemes\":[]}",  kb);

        /*if(!input_port.open("/"+robotName+"/iDevER:i"))
        {
            printf("Failed creating input port for DevER module");
            return false;
        }

        if(!output_port.open("/"+robotName+"/DevER:or"))
        {
            printf("Failed creating port for body controler (/%s/DevER:or)",robotName);
            return false;
        }*/




          local = "/"+robotName+"/DevER:or";
         remote = "/"+robotName+"/bodyController:ir";

        if(Network::connect(local,remote))
        {
            yInfo(" Stablished  port connection between %s and /%s",local.c_str(),remote.c_str());
        }
        else
        {
            yWarning(" Failed stablishing connection between %s and %s. Is the bodyController module running?\n",local.c_str(),remote.c_str());

        }

        jreader.parse("{}",  perceptualMemory);

        return true;
    }

    virtual void afterStart(bool s)
    {
        if (s)
            printf("Perception module started successfully\n");
        else
            printf("Perception module did not start\n");
    }

    virtual void run()
    {

        ImageOf<PixelBgr> *inputImage = inputImagePort.read();  // read an image
        ImageOf<PixelBgr> &outputImage = outputImagePort.prepare(); //get an output image
        Bottle outputAttendedContext;// = outputAttendedContextPort.prepare(); //get an output image
        //outContext.clear();
        //outContext.color="c1";
//outContext.size="s1";
//outContext.moving=true;

        outputImage.resize(inputImage->width(), inputImage->height());


        IplImage *tmp_ipl_image = (IplImage*) inputImage->getIplImage();
        Mat imgMat = cvarrToMat(tmp_ipl_image);
        Mat im_with_keypoints = imgMat;

        int nblobs = 0;
        Json::Value perceptualContext;

        jreader.parse("[{},{}]",  perceptualContext); // initialization

        bool attending = false;
        for (Json::Value color : perceptualMemory["colors"])
        {
            for (Json::Value area : perceptualMemory["sizes"])
            {

                //printf("Detecting objects of color:%s and size:%s\n",color["name"].asString().c_str(),area["name"].asString().c_str());
//

                SimpleBlobDetector::Params params;

                // Change thresholds
                params.minThreshold = 30;
                params.maxThreshold = 200;

                // Filter by Area.
                params.filterByArea = true;
                //printf("area between %d - %d" ,area.from,area.to);
                params.minArea = area["from"].asDouble();
                params.maxArea = area["to"].asDouble();;

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

               // std::cout << "Color: "<< color.toStyledString() << std::endl;

                cv::Scalar from = cv::Scalar(color["from"][0].asInt(),color["from"][1].asInt(),color["from"][2].asInt());
                cv::Scalar to = cv::Scalar(color["to"][0].asInt(),color["to"][1].asInt(),color["to"][2].asInt());


                inRange(imgMat, from, to, color_filtered_img);

                std::vector<KeyPoint> keypoints;
                detector.detect(color_filtered_img, keypoints);

                for (std::vector<KeyPoint>::const_iterator i = keypoints.begin(); i != keypoints.end(); ++i)
                {

                    printf("Detected something\n");
                string objId = "obj"+to_string(++nblobs);
                perceptualContext.append(objId.c_str());

                    int xpos = floor(i->pt.x / 106);
                    int ypos = floor(i->pt.y / 80);
                    //rapidjson::Value object(rapidjson::kObjectType);


                    Json::Value perceivedObj;
                    perceivedObj.append("color");

                    perceivedObj["color"]= color["name"].asString();

                    //Json::Value perceivedSize;
                    perceivedObj.append("size");
                    perceivedObj["size"]= area["name"].asString();
                    //sizev.SetString(area.name.c_str(), allocator);

                    //object.AddMember("color", colorv, allocator);
                    //object.AddMember("size", sizev, allocator);


                    //Json::Value objName;
                    //perceivedObj.SetString(("obj"+to_string(++nblobs)).c_str(), allocator);

                    perceivedObj.append("sector");
                    perceivedObj["sector"] = 3*ypos+xpos+1;
                    perceptualContext[0][objId] = perceivedObj;
                }
                drawKeypoints( im_with_keypoints, keypoints, im_with_keypoints, Scalar(0,255,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );

            }

        }

        printf("for ended\n");
        //Json::Value *.asDouble();;
        Json::Value attendedContext;

        jreader.parse("[{},{}]",  attendedContext);

        string attendedObjId ="";
        //Json::Value *attendedContext;
        if(nblobs>0)
        {
            int att = (rand() % nblobs)+1;
            attendedObjId= "obj"+std::to_string(att);
            attendedContext[0] = perceptualContext[attendedObjId.c_str()];
        }

        /*StringBuffer strbuf;
        Writer<StringBuffer> writer(strbuf);
        document.Accept(writer);*/

        std::cout << "Perceptual  context: "<< perceptualContext.toStyledString() << std::endl;
std::cout << "Attended  context: "<< attendedContext.toStyledString() << std::endl;

        line(im_with_keypoints, Point(0,80), Point(inputImage->width(),80), Scalar(0, 255, 0),1,8,0);
        line(im_with_keypoints, Point(0,160), Point(inputImage->width(),160), Scalar(0, 255, 0),1,8,0);
        line(im_with_keypoints, Point(106,0), Point(106,inputImage->height()), Scalar(0, 255, 0),1,8,0);
        line(im_with_keypoints, Point(212,0), Point(212,inputImage->height()), Scalar(0, 255, 0),1,8,0);



        IplImage blobed = IplImage( im_with_keypoints );
        ImageOf<PixelBgr> yarpReturnImage;
        yarpReturnImage.wrapIplImage(&blobed);
        outputImage=yarpReturnImage;

        outputImagePort.write();

            printf("Sending context to emotion module  \n");
            outputAttendedContext.addString(attendedContext.toStyledString());

            outputContextPort.write(outputAttendedContext);

            printf("Perception: waiting for emotion \n");
           /* Bottle input;
            inputEmotionPort.read(input);
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
            outputAttendedContextPort.write(att_context);

            printf("PM: Waiting for DevER to perform action\n");
            Bottle done;
            //propio_port.read(done);

            printf("Got %s\n",done.toString().c_str());
*/

        }


    virtual void threadRelease()
    {
        printf("Stoping perception module...\n");

    }



    bool loadPerceptualMemory(string perceptualMemoryPath){
            // Load jacub's knowledge base (schemes)
        //std::ifstream ifs { R"("++")" };
        printf("Loading %s\n",perceptualMemoryPath.c_str());
        std::ifstream ifs;
 ifs.open(perceptualMemoryPath);
        //std::ifstream ifs("("+kb_file+")" );
        if ( !ifs.is_open() )
        {
            std::cerr << "Could not load perceptual memory "<<perceptualMemoryPath<<"\n";
            return EXIT_FAILURE;
        }

        Json::Reader reader;
        //Json::Value kb; // array of schemas
        reader.parse(ifs, perceptualMemory);

        //std::cout << kb["schemes"].toStyledString() << '\n';

        // toStyledString	(		 )
        //totalSchemes = kb["schemes"].size();

        std::cout << perceptualMemory.toStyledString() << '\n';

        std::cout << " Perceptual memory loaded [ok]" << '\n';
        return true;
    }

};

class Perception: public RFModule
{
protected:
    PerceptionThread *perceptionThr;

public:
    virtual bool configure(ResourceFinder &rf)
    {

    string robotName = rf.check("robot",Value("jacub")).asString();
    //string perceptual_memory_path
        //string robotName = rf.find("robot").asString();
        printf("Encontró nombre de robot: %s\n",robotName.c_str());

        Time::turboBoost();
        perceptionThr=new PerceptionThread(robotName, CTRL_THREAD_PER);
        if (!perceptionThr->start())
        {
            delete perceptionThr;
            return false;
        }

        perceptionThr->loadPerceptualMemory(rf.check("perceptual_memory_path",Value("../../schemas/perceptualMemory.json")).toString());

        return true;
    }

    virtual bool close()
    {
        perceptionThr->stop();
        delete perceptionThr;

        return true;
    }

    virtual double getPeriod()
    {
        return 1.0;
    }
    virtual bool   updateModule()
    {
        return true;
    }
};


int main(int argc, char *argv[])
{

    Network yarp;

    if (!yarp.checkNetwork())
    {
        yError("Yarp server does not seem available\n");
        return 1;
    }


    Perception perception;

    ResourceFinder rf;
rf.setVerbose(); //logs searched directories
rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
rf.configure(argc,argv);
//idever.configure(rf);

string robotName = rf.check("robot",Value("jacub")).asString();

/*
string  local = "/"+robotName+"/DevER:or";
string remote = "/"+robotName+"/bodyController:ir";

    if(Network::connect(local,remote))
    {
        yError(" Yarp server does not seem available\n");
        yInfo(" Stablished  port connection between %s and /%s",local.c_str(),remote.c_str());
    }
    else
    {
        yWarning(" Failed stablishing connection between %s and %s. Is the bodyController module running?\n",local.c_str(),remote.c_str());

    }
    */

    perception.runModule(rf);
}

