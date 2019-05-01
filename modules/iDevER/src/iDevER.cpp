 // -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
 //
 // A tutorial on how to use the Cartesian Interface to control a limb
 // in the operational space.
 //
 // Author: Ugo Pattacini - <ugo.pattacini@iit.it>

 #include <cstdio>
 #include <cmath>

 #include <yarp/os/Network.h>
 #include <yarp/os/RFModule.h>
 #include <yarp/os/RateThread.h>
 #include <yarp/os/Time.h>
 #include <yarp/sig/Vector.h>
 #include <yarp/math/Math.h>
#include <yarp/os/all.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/filereadstream.h"
#include <rapidjson/istreamwrapper.h>

#include <fstream>
#include <iostream>

 #define CTRL_THREAD_PER     0.02    // [s]
 #define PRINT_STATUS_PER    1.0     // [s]
 #define MAX_TORSO_PITCH     30.0    // [deg]

 using namespace std;
using namespace rapidjson;
using namespace yarp::os;

 class DevERThread: public RateThread
 {

     rapidjson::Document LTM; //Long term memory (esquemas aprendidos)

        Port input_port;
        Port output_port;
     // the event callback attached to the "motion-ongoing"

 public:
    DevERThread(const double period) : RateThread(int(period*1000.0))
     {
         // we wanna raise an event each time the arm is at 20%
         // of the trajectory (or 80% far from the target)
         //cartesianEventParameters.type="motion-ongoing";
         //cartesianEventParameters.motionOngoingCheckPoint=0.2;
     }

     virtual bool threadInit()
     {
      //ifstream inFile;
      //inFile.open("../schemas/learned_schemas.json");
        /*FILE* fp = fopen("../../schemas/learned.json", "rb"); // non-Windows use "r"
        char readBuffer[65536];
        FileReadStream is(fp, readBuffer, sizeof(readBuffer));

        LTM.ParseStream(is);
        StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	LTM.Accept(writer);*/


/*cout << "Visual context: "<< strbuf.GetString() << endl;	   /*if(!port.open("/memory/in")){
            printf("Failed creating port for memory");
            return false;
       }*/

       std::ifstream ifs { R"(../../schemas/learned.json)" };
    if ( !ifs.is_open() )
    {
        std::cerr << "Could not load ../../schemas/learned.json!\n";
        return EXIT_FAILURE;
    }

    IStreamWrapper isw { ifs };

    Document doc {};
    doc.ParseStream( isw );

    StringBuffer buffer {};
    Writer<StringBuffer> writer { buffer };
    doc.Accept( writer );

    if (doc.HasParseError() )
    {
        std::cout << "Error  : " << doc.GetParseError()  << '\n'
                  << "Offset : " << doc.GetErrorOffset() << '\n';
        return EXIT_FAILURE;
    }

    const std::string jsonStr { buffer.GetString() };

    std::cout << jsonStr << '\n';


       if(!input_port.open("/jacub/DevER")){
            printf("Failed creating input port for DevER module");
            return false;
       }

       if(!output_port.open("/jacub/DevER/out")){
            printf("Failed creating output port for DevER module");
            return false;
       }

       return true;



     }

     virtual void afterStart(bool s)
     {
         if (s)
             printf("DevER thread started successfully\n");
         else
             printf("DevER thread did not start\n");

         //t=t0=t1=Time::now();
     }

     virtual void run()
     {


         //while (true) {
        printf("waiting for input \n");
        Bottle input;
        input_port.read(input);
        //if (input!=NULL) {
            printf("got %s\n",input.toString().c_str());
           rapidjson::Document attendedctx;

            attendedctx.SetObject();
            string input_context = input.toString();

            input_context.erase(0,1);
            input_context.erase(input_context.size()-1,input_context.size());
            size_t pos;
            //string "\\";
	while ((pos = input_context.find("\\")) != std::string::npos) {
		input_context.replace(pos, 1, "");
	}

	printf("CLEANSED =Z=> %s\n",input_context.c_str());

	if (attendedctx.Parse<0>(input_context.c_str()).HasParseError()){
                printf("EM: Error trying to parse input\n");
                return;
            }


            StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	attendedctx.Accept(writer);

	std::cout << "DevER: received attended context \n"<< strbuf.GetString() << std::endl;


    }

     virtual void threadRelease()
     {
         // we require an immediate stop
         // before closing the left_arm for safety reason

         printf("Killing memory...\n");
     }
 };



 class DevER: public RFModule
 {
 protected:
     DevERThread *thr;

 public:
     virtual bool configure(ResourceFinder &rf)
     {
         Time::turboBoost();

         thr=new DevERThread(CTRL_THREAD_PER);
         if (!thr->start())
         {
             delete thr;
             return false;
         }

         return true;
     }

     virtual bool close()
     {
         thr->stop();
         delete thr;

         return true;
     }

     virtual double getPeriod()    { return 1.0;  }
     virtual bool   updateModule() { return true; }
 };



 int main()
 {
     Network yarp;

     if (!yarp.checkNetwork())
     {
         printf("Error: yarp server does not seem available\n");
         return 1;
     }

     DevER emo;

     ResourceFinder rf;

     emo.runModule(rf);

     //return Network::connect("/perception/context/out","/memory/in");
    // return 0;
 }

