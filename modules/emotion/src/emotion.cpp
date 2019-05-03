 // -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
 //
 // A tutorial on how to use the Cartesian Interface to control a limb
 // in the operational space.
 //
 // Author: Ugo Pattacini - <ugo.pattacini@iit.it>

 #include <cstdio>
 #include <cmath>
#include <iostream>
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


 #define CTRL_THREAD_PER     0.02    // [s]
 #define PRINT_STATUS_PER    1.0     // [s]
 #define MAX_TORSO_PITCH     30.0    // [deg]

 using namespace std;
using namespace yarp::os;
using namespace rapidjson;

 class EmotionThread: public RateThread
 {

     rapidjson::Document STM;

        Port input_port;
        Port output_port;
        Port emo_per_port;
     // the event callback attached to the "motion-ongoing"

 public:
    EmotionThread(const double period) : RateThread(int(period*1000.0))
     {
         // we wanna raise an event each time the arm is at 20%
         // of the trajectory (or 80% far from the target)
         //cartesianEventParameters.type="motion-ongoing";
         //cartesianEventParameters.motionOngoingCheckPoint=0.2;
     }

     virtual bool threadInit()
     {
     //moved = false;
         // open a left_arm interface to connect to the cartesian server of the simulator
         // we suppose that:
         //
         // 1 - the iCub simulator is running
         //     (launch: iCub_SIM)
         //
         // 2 - the cartesian server is running
         //     (launch: yarprobotinterface --context simCartesianControl)
         //
         // 3 - the cartesian solver for the left arm is running too
         //     (launch: iKinCartesianSolver --context simCartesianControl --part left_arm)
         //


	   /*if(!port.open("/memory/in")){
            printf("Failed creating port for memory");
            return false;
       }*/

       if(!input_port.open("/jacub/emotion/context/in")){
            printf("Failed creating input port for emotion module");
            return false;
       }

       if(!output_port.open("/jacub/emotion/out")){
            printf("Failed creating output port for emotion module");
            return false;
       }

       if(!emo_per_port.open("/jacub/emotion/perception")){
            printf("Failed creating /jacub/emotion/perception/out port");
            return false;
       }

       return true;



     }

     virtual void afterStart(bool s)
     {
         if (s)
             printf("Emotion thread started successfully\n");
         else
             printf("Emotion thread did not start\n");

         //t=t0=t1=Time::now();
     }

     virtual void run()
     {
         //while (true) {
        printf("Emotion: waiting for input \n");
        Bottle input;
        emo_per_port.read(input);
        //if (input!=NULL) {
            printf("Emotion: Got %s\n",input.toString().c_str());

            rapidjson::Document attendedobj;

            attendedobj.SetObject();
            string input_context = input.toString();

            input_context.erase(0,1);
            input_context.erase(input_context.size()-1,input_context.size());
            size_t pos;
            //string "\\";
	while ((pos = input_context.find("\\")) != std::string::npos) {
		input_context.replace(pos, 1, "");
	}


	/*while ((pos = input_context.find("\"{")) != std::string::npos) {
		input_context.replace(pos, 1, "{");
	}
	while ((pos = input_context.find("\"}")) != std::string::npos) {
		input_context.replace(pos, 1, "}");
	}*/
	// create a rapidjson array type with similar syntax to std::vector
	//rapidjson::Value objs(rapidjson::kArrayType);

	// must pass an allocator when the object may need to allocate memory

            if (attendedobj.Parse<0>(input_context.c_str()).HasParseError()){
                printf("EM: Error trying to parse input\n");
                return;
            }


            StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	attendedobj.Accept(writer);

	std::cout << "EM: received attended context "<< strbuf.GetString() << std::endl;

	string gemotion =  "hap";

	if(attendedobj.HasMember("color")){
	//printf("tiene miembro!!\n");

		//printf("COLOR => %s",attendedobj.color);
           // assert(attendedobj["color"].IsString());

           rapidjson::Value& color = attendedobj["color"];
//printf("holis");
	//printf("COLOR => %s",color.GetString());


            if(strcmp(color.GetString(), "red")==0){
                gemotion = "sad";
            }////

            }
           /* double total = 0;
            for (int i=0; i<input.size(); i++) {
                total += input.get(i).asInt64();
            }
            //Bottle output;// = port.prepare();
            //output.clear();
            //output.addString("total");
            //output.addInt64(total);
            printf("total %i",total);
            //port.write();
        }*/
       // port.
        //input->clear();
        //printf("endedn runing\n");

        Bottle emotioncmd;
Bottle response;
        emotioncmd.addString("set");
        emotioncmd.addString("all");
        emotioncmd.addString(gemotion.c_str());

        output_port.write(emotioncmd,response);

        //printf("emotional response  %s\n",response.toString().c_str());

        printf("Generated emotion: %s\n",gemotion.c_str());

        //Send generated emotion to  perseption module
        Bottle emotion;
        emotion.addString(gemotion.c_str());
        emo_per_port.write(emotion);
   // _emotioncmd.addString("set");
    }

     virtual void threadRelease()
     {
         // we require an immediate stop
         // before closing the left_arm for safety reason

         printf("Killing memory...\n");
     }
 };



 class Emotion: public RFModule
 {
 protected:
     EmotionThread *thr;

 public:
     virtual bool configure(ResourceFinder &rf)
     {
         Time::turboBoost();

         thr=new EmotionThread(CTRL_THREAD_PER);
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

     Emotion emo;

     ResourceFinder rf;

     emo.runModule(rf);

     //return Network::connect("/perception/context/out","/memory/in");
    // return 0;
 }
