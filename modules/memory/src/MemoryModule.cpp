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


 #define CTRL_THREAD_PER     0.02    // [s]
 #define PRINT_STATUS_PER    1.0     // [s]
 #define MAX_TORSO_PITCH     30.0    // [deg]

 using namespace std;
using namespace yarp::os;

 class MemoryThread: public RateThread
 {

     rapidjson::Document STM;

        Port port;
        Port emotion_port;
     // the event callback attached to the "motion-ongoing"

 public:
     MemoryThread(const double period) : RateThread(int(period*1000.0))
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


	   if(!port.open("/memory/in")){
            printf("Failed creating port for memory");
            return false;
       }

       if(!emotion_port.open("/jacub/emotion/out")){
            printf("Failed creating emotional port for memory");
            return false;
       }

       return true;



     }

     virtual void afterStart(bool s)
     {
         if (s)
             printf("Memory thread started successfully\n");
         else
             printf("Memory thread did not start\n");

         //t=t0=t1=Time::now();
     }

     virtual void run()
     {
         //while (true) {
        printf("waiting for input \n");
        Bottle input;
        port.read(input);
        //if (input!=NULL) {
            printf("got %s\n",input.toString().c_str());
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
        emotioncmd.addString("sad");

        emotion_port.write(emotioncmd,response);

        printf("emotional response  %s\n",response.toString().c_str());

        printf("Generated emotion: sadness\n");
   // _
    }

     virtual void threadRelease()
     {
         // we require an immediate stop
         // before closing the left_arm for safety reason

         printf("Killing memory...\n");
     }
 };



 class Memory: public RFModule
 {
 protected:
     MemoryThread *thr;

 public:
     virtual bool configure(ResourceFinder &rf)
     {
         Time::turboBoost();

         thr=new MemoryThread(CTRL_THREAD_PER);
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

     Memory mem;

     ResourceFinder rf;

     mem.runModule(rf);

     //return Network::connect("/perception/context/out","/memory/in");
    // return 0;
 }
