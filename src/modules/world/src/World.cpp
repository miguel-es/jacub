 // -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
 //
 // A tutorial on how to use the Cartesian Interface to control a limb
 // in the operational space.
 //
 // Author: Ugo Pattacini - <ugo.pattacini@iit.it>

 #include<iostream>
 //#include <cstdio>
 #include <cmath>

 #include <yarp/os/Network.h>
 #include <yarp/os/RFModule.h>
 #include <yarp/os/RateThread.h>
 #include <yarp/os/RpcClient.h>
#include <yarp/os/impl/Logger.h>


 #define CTRL_THREAD_PER     0.02    // [s]

 using namespace std;
 using namespace yarp::os;


 class WorldCtrlThread: public RateThread
 {

     RpcClient world_port;
     Port continueInputPort;
     bool inited;
     int step;
	 double x;
	 double y;
	 double z;
	 int delta;

     // the event callback attached to the "motion-ongoing"

 public:
     WorldCtrlThread(const double period) : RateThread(int(period*1000.0)){}

     virtual bool threadInit()
     {

    	 	step = 0;
    	  x= 0.0f;
    	  y = 0.55387995f;
    	  z = 0.60f;
    	  delta = 1;

       inited = false;
	   if(!world_port.open("/jacub/world")){
            yError(" World: Failed creating port");
            return false;
       }

       if(!world_port.addOutput("/icubSim/world")){
            yError(" World: Failed adding output to /icubSim/word port. Is iCub_SIM running?");
            return false;
       }

       if(!continueInputPort.open("/world/continue:i")){
                   yError(" World: Failed adding input port /world/continue:i");
                   return false;
              }


       Bottle cmd;
       	  cmd.addString("world");
       	  cmd.addString("del");
       	  cmd.addString("all");

       	yInfo(" World: Cleaning world\n");
       		Bottle response;
       		world_port.write(cmd,response);
       		yDebug(" World: response: %s",response.toString().c_str());

       		//addObj("box",0.06f,0.06f,0.06f,0.0f,0.55387995f,0.37f,1,0,0);
       	//	addObj("box",0.06f,0.06f,0.06f,0.2f,0.55387995f,0.35f,1,0,0);
       		//addObj("box",0.06f,0.06f,0.06f,0.09f,0.55387995f,0.35f,1,0,0);
       		//addObj("box",0.06f,0.06f,0.06f,0.00f,0.55387995f,0.35f,1,1,0);
       		getPos("box",1);
       return true;
     }

     virtual void afterStart(bool s)
     {
         if (s)
             yInfo("World control thread started successfully\n");
         else
             yError("World control thread did not start\n");

     }

     virtual void run()
     {

    	/*if(step==15){
    		yDebug(" World: Adding green box");
    		mvObj("box",0.2f,0.55387995f,0.35f);
    		step++;
    		//addObj("box",0.06f,0.06f,0.06f,0.2f,0.55387995f,0.35f,0,1,0);
    	}*/
    	 Bottle input;
    	 continueInputPort.read(input);
    	 //if(delta==1 && x<0.325000){
    	// 0.0900000035762787
    		// if(x>0.325000) return;
       /*  yDebug(" step left=%f ",x);
       x=x+(delta*0.03);
         yDebug("x'=%f\n",x);
         mvObj("box",x,y,z);
         step++;
*/

    	 //}
       /*  if(x>0.325000) delta = -1;
         if(x<-0.325000) delta = 1;*/
    	 /*else if(step<600 && x>0){
             yDebug("step right=%f ",x);
             x-=0.005;
             yDebug("X'=%f\n",x);
             mvObj("box",x,y,z);
             //step++;
    	 }
    	 step++;
    	 if(step==600){
    		 step = 0;
    	 }*/

     }

     virtual void threadRelease()
     {

         world_port.close();
     }

      void mvObj(string obj,float x,float y,float z){
	  Bottle cmd;
	  cmd.addString("world");
	  cmd.addString("set");
	  cmd.addString(obj);
	  cmd.addInt(1);

	  cmd.addDouble(x);
	  cmd.addDouble(y);
	  cmd.addDouble(z);
	yDebug("Moving %s\n",obj.c_str());
	Bottle response;
	world_port.write(cmd,response);
	yDebug(" World: response: %s",response.toString().c_str());
    }

      void addObj(string obj,float size1,float size2,float size3,float x,float y,float z, int r, int g, int b){
      	  Bottle cmd;
      	  cmd.addString("world");
      	  cmd.addString("mk");
      	  cmd.addString(obj);
      	  if(size1>=0) cmd.addDouble(size1);
      	  if(size2>=0) cmd.addDouble(size2);
      	  if(size3>=0) cmd.addDouble(size3);
      	  cmd.addDouble(x);
      	  cmd.addDouble(y);
      	  cmd.addDouble(z);
      	  cmd.addInt(r);
      	  cmd.addInt(g);
      	  cmd.addInt(b);
      	printf("Adding %s to the world\n",obj.c_str());
      	Bottle response;
      	world_port.write(cmd,response);
      	printf("World port response: %s\n",response.toString().c_str());
          }

      void getPos(string obj,int id){
    	  Bottle cmd;
    	        	  cmd.addString("world");
    	        	  cmd.addString("get");
    	        	  cmd.addString(obj);
    	        	  cmd.addInt(1);
    	        	  printf("Getting position of %s %d\n",obj.c_str(),id);
    	        	  Bottle response;
    	        	        	world_port.write(cmd,response);
    	        	        	printf("World port response: %s\n",response.toString().c_str());
    	        	        	z = response.pop().asDouble();
    	        	        	printf("GETTED Z = %f\n",z);
    	        	        	y = response.pop().asDouble();
    	        	        	printf("GETTED Y = %f\n",y);
    	        	        	x = response.pop().asDouble();
    	        	        	printf("GETTED X = %f\n",x);
      }

void import3DModel(string xmodel,string texture,float x,float y, float z){
    printf("Importing %s\n",xmodel.c_str());
	Bottle cmd;
	cmd.addString("world");
	cmd.addString("mk");
	cmd.addString("smodel");
	cmd.addString(xmodel);
	cmd.addString(texture);
	cmd.addDouble(x);
	cmd.addDouble(y);
	cmd.addDouble(z);
	Bottle response;
	world_port.write(cmd,response);
	printf("World port response: %s\n",response.toString().c_str());
	}

 };



 class World: public RFModule
 {
 protected:
     WorldCtrlThread *thr;

 public:
     virtual bool configure(ResourceFinder &rf)
     {
         //Time::turboBoost();

         thr=new WorldCtrlThread(CTRL_THREAD_PER);
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

     World world;

     ResourceFinder rf;

     return world.runModule(rf);
 }
