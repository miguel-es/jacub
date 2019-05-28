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

#include <jsoncpp/json/json.h>

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

        int totalSchemes;
        bool partialMatch;
        Document kb;
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

       partialMatch = false;

    // Load jacub's knowledge base (schemes)
       std::ifstream ifs { R"(../../schemas/kb.json)" };
    if ( !ifs.is_open() )
    {
        std::cerr << "Could not load knokledge base ../../schemas/kb.json!\n";
        return EXIT_FAILURE;
    }

    IStreamWrapper isw { ifs };

     //kb;
    kb.ParseStream( isw );

    StringBuffer buffer {};
    Writer<StringBuffer> writer { buffer };
    kb.Accept( writer );

    if (kb.HasParseError() )
    {
        std::cout << "Error  : " << kb.GetParseError()  << '\n'
                  << "Offset : " << kb.GetErrorOffset() << '\n';
        return EXIT_FAILURE;
    }

    const std::string jsonStr { buffer.GetString() };

    std::cout << jsonStr << '\n';

    totalSchemes = kb["schemes"].GetArray().Size();
        std::cout << totalSchemes << " loaded schemas" << '\n';

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
                Document attendedContext;
        //if (input!=NULL) {
            printf("got %s\n",input.toString().c_str());

            attendedContext.SetObject();
            string input_context = input.toString();

            input_context.erase(0,1);
            input_context.erase(input_context.size()-1,input_context.size());
            size_t pos;
            //string "\\";
	while ((pos = input_context.find("\\")) != std::string::npos) {
		input_context.replace(pos, 1, "");
	}

	if (attendedContext.Parse<0>(input_context.c_str()).HasParseError()){
                printf("EM: Error trying to parse input\n");
                return;
            }


            StringBuffer strbuf;
	Writer<StringBuffer> writer(strbuf);
	attendedContext.Accept(writer);

	std::cout << "DevER: received attended context \n"<< strbuf.GetString() << std::endl;

	string action =  engagement(&attendedContext);
//string action ="";

Json::Reader reader;
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
//printf(" Creating bottle \n");
        Bottle actioncmd;
Bottle response;
        actioncmd.addString(action.c_str());


        printf("iDevER: taken action: %s\n",action.c_str());
        output_port.write(actioncmd);


        //printf("emotional response  %s\n",response.toString().c_str());

        //printf("iDevER: taken action: mv lhnd %s\n",actiondir.c_str());

        //Send generated emotion to  perseption module


    }


     virtual void threadRelease()
     {
         // we require an immediate stop
         // before closing the left_arm for safety reason

         printf("Killing memory...\n");
     }

    string engagement(Document* context){

     if(partialMatch){

     }else{ //try a 100% matching
        printf("Engagement: trying partial match");
        for(auto& schema: kb["schemas"].GetArray()){
            printf("holi iterando");
        }

     }

     string action = "mvlhndr";

	if(context->HasMember("color")){
	//printf("tiene miembro!!\n");

		//printf("COLOR => %s",attendedobj.color);
           // assert(attendedobj["color"].IsString());

          /*rapidjson::Value& color = context->color;
//printf("holis");
	//printf("COLOR => %s",color.GetString());


            if(strcmp(color.GetString(), "red")==0){
                action = "mvlhndl";
            }////

            }*/

          return action;
     }
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

