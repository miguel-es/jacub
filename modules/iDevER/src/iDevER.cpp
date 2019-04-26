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

 class DevERThread: public RateThread
 {

     rapidjson::Document STM;

        Port port;
        Port DevER_port;
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

       if(!DevER_in.open("/jacub/DevER/in")){
            printf("Failed creating input port for DevER module");
            return false;
       }

       if(!DevER_out.open("/jacub/DevER/out")){
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

        Bottle DevERcmd;
Bottle response;
        DevERcmd.addString("set");
        DevERcmd.addString("all");
        DevERcmd.addString("sad");

        DevER_port.write(DevERcmd,response);

        printf("DevERal response  %s\n",response.toString().c_str());

        printf("Generated DevER: sadness\n");
   // _
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

     DevER emo;

     ResourceFinder rf;

     emo.runModule(rf);

     //return Network::connect("/perception/context/out","/memory/in");
    // return 0;
 }

