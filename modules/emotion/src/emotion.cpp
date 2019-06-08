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

#include <jsoncpp/json/json.h>
#include <jutils.cpp>

 #define CTRL_THREAD_PER     0.02    // [s]
 #define PRINT_STATUS_PER    1.0     // [s]
 #define MAX_TORSO_PITCH     30.0    // [deg]

 using namespace std;
using namespace yarp::os;

 class EmotionThread: public RateThread
 {
        Port inputContextPort;
        Port outputEmotionalContextPort;
        Port outputEmotionPort;
        bool bored;
        bool interested;
        bool surprised;
     // the event callback attached to the "motion-ongoing"

 public:
    EmotionThread(const double period) : RateThread(int(period*1000.0))
     {
         // we wanna raise an event each time the arm is at 20%
         // of the trajectory (or 80% far from the target)
         //cartesianEventParameters.type="motion-ongoing";
         //cartesianEventParameters.motionOngoingCheckPoint=0.2;
        bored = false;
        interested = false;
        surprised = false;
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

       if(!inputContextPort.open("/jacub/emotion/context/in")){
            printf("Failed creating input port for emotion module");
            return false;
       }

       if(!outputEmotionalContextPort.open("/jacub/emotion/emotionalContext:o")){
            printf("Failed creating output port for emotion module");
            return false;
       }

       if(!outputEmotionPort.open("/jacub/emotion/perception")){
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
        inputContextPort.read(input);



        Json::Reader reader;
        Json::Value attendedContext;

        //printf("got input %s ----- \n",input.toString().c_str());
        string input_string = input.toString();
        prepareInput(input_string);

        reader.parse(input_string.c_str(),  attendedContext);

        std::cout << "Got context: " << '\n' << attendedContext.toStyledString() << '\n';

        std::cout << "Got context: " << '\n' << attendedContext[0].toStyledString() << '\n';

	/*while ((pos = input_context.find("\"{")) != std::string::npos) {
		input_context.replace(pos, 1, "{");
	}
	while ((pos = input_context.find("\"}")) != std::string::npos) {
		input_context.replace(pos, 1, "}");
	}*/
	// create a rapidjson array type with similar syntax to std::vector
	//rapidjson::Value objs(rapidjson::kArrayType);

	// must pass an allocator when the object may need to allocate memory

            /*if (attendedobj.Parse<0>(input_context.c_str()).HasParseError()){
                printf("EM: Error trying to parse input\n");
                return;
            }


            StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	attendedobj.Accept(writer);

	std::cout << "EM: received attended context "<< strbuf.GetString() << std::endl;
*/
	//int gemotion =  1;
	int pleasure = 1;
	if(attendedContext[0].empty()){
	printf("entro1\n");
       // ((Json::objectValue) attendedContext[0]).append("sad");
        //gemotion = "sad";
                pleasure = -1;

    }else if(!attendedContext[0]["sector"].empty() && attendedContext[0]["sector"]==5){//if the object is centered
            printf("Object centered entro2\n");
            pleasure = 2;
    }

        attendedContext[0]["pleasure"] = pleasure;// = -1;
        std::cout << "Got context3: " << '\n' << attendedContext[0].toStyledString() << '\n';

       if(attendedContext[1].empty()){
            attendedContext[1]["pleasure"]=-1;
       // attendedContext[1] = -1;
       }else{
               attendedContext[1]["pleasure"]=1;
       }

string gemotion = "hap";
        Bottle emotioncmd;
Bottle response;
        emotioncmd.addString("set");
        emotioncmd.addString("all");
        emotioncmd.addString(gemotion.c_str());

        outputEmotionPort.write(emotioncmd,response);

        //printf("emotional response  %s\n",response.toString().c_str());

        std::cout << "Emotional context: "<< attendedContext.toStyledString() << std::endl;

        //Send generated emotion to  perseption module
       // Bottle response;

       Json::FastWriter fastWriter;
std::string output = fastWriter.write(attendedContext);

        response.addString(output);
        outputEmotionalContextPort.write(response);
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
