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


 #define CTRL_THREAD_PER     0.02    // [s]

 using namespace std;
 using namespace yarp::os;


 class WorldCtrlThread: public RateThread
 {

     RpcClient world_port;
     bool inited;

	 double x;
	 double y;
	 double z;

     // the event callback attached to the "motion-ongoing"

 public:
     WorldCtrlThread(const double period) : RateThread(int(period*1000.0)){}

     virtual bool threadInit()
     {

    	  x= 0.09f;
    	  y = 0.55387995f;
    	  z = 0.35f;

       inited = false;
	   if(!world_port.open("/jacub/world")){
            printf("Failed creating port");
            return false;
       }

       if(!world_port.addOutput("/icubSim/world")){
            printf("Failed adding output to /icubSim/word port. Is iCub_SIM running?");
            return false;
       }

       Bottle cmd;
       	  cmd.addString("world");
       	  cmd.addString("del");
       	  cmd.addString("all");

       	printf("Cleaning world\n");
       		Bottle response;
       		world_port.write(cmd,response);
       		printf("World  response: %s",response.toString().c_str());

       		addObj("box",0.06f,0.06f,0.06f,0.09f,0.55387995f,0.35f,0,0,1);
       		getPos("box",1);
       return true;
     }

     virtual void afterStart(bool s)
     {
         if (s)
             printf("World control thread started successfully\n");
         else
             printf("World control thread did not start\n");

     }

     virtual void run()
     {
    	// 0.0900000035762787
         printf("X=%f ",x);
         x+=0.001;
         printf("X'=%f\n",x);
         mvObj("box",x,y,z);

         if(x>0.225000) x = 0.09;

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
	printf("Moving %s\n",obj.c_str());
	Bottle response;
	world_port.write(cmd,response);
	printf("World  response: %s",response.toString().c_str());
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
