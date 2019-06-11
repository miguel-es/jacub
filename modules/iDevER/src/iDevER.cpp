// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Cartesian Interface to control a limb
// in the operational space.
//
// Author: Ugo Pattacini - <ugo.pattacini@iit.it>

#include <cstdio>
#include <cmath>

#include <jutils.cpp>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>
#include <yarp/os/all.h>

#include <jsoncpp/json/json.h>

#include <yarp/os/impl/Logger.h>

#include <fstream>
#include <iostream>

#define CTRL_THREAD_PER     0.02    // [s]
#define PRINT_STATUS_PER    1.0     // [s]
#define MAX_TORSO_PITCH     30.0    // [deg]

using namespace std;
using namespace yarp::os;
using namespace yarp::os::impl;

class DevERThread: public RateThread
{

    Port inputContextPort;
    Port output_port;
    Port outputExpectationPort;

    int totalSchemes;
    bool allowPartialMatch;

    bool accommodated;
    Json::Value kb;

    Json::Value matchedSchema;

    string robotName;
    // the event callback attached to the "motion-ongoing"

public:
    DevERThread(string robotName, const double period) : RateThread(int(period*1000.0))
    {
        // we wanna raise an event each time the arm is at 20%
        // of the trajectory (or 80% far from the target)
        //cartesianEventParameters.type="motion-ongoing";
        //cartesianEventParameters.motionOngoingCheckPoint=0.2;
        this->robotName = robotName;

    }

    virtual bool threadInit()
    {


        allowPartialMatch = true;


        /* ifstream ifs("../../schemas/kb.json");
        Json::Reader reader;
        Json::Value obj;
        reader.parse(ifs, obj);*/





        Json::Reader reader;
        reader.parse("{\"schemes\":[]}",  kb);

        if(!inputContextPort.open("/"+robotName+"/iDevER/context:i"))
        {
            printf("Failed creating input port for DevER module");
            return false;
        }

        if(!outputExpectationPort.open("/"+robotName+"/iDevER/expectation:o"))
        {
            printf("Failed creating output port for expectation");
            return false;
        }


        if(!output_port.open("/"+robotName+"/DevER:or"))
        {
            printf("Failed creating port for body controler (/%s/DevER:or)",robotName);
            return false;
        }

        string  local = "/"+robotName+"/DevER:or";
        string remote = "/"+robotName+"/bodyController:ir";

        if(Network::connect(local,remote))
        {
            yInfo(" Stablished  port connection between %s and /%s",local.c_str(),remote.c_str());
        }
        else
        {
            yWarning(" Failed stablishing connection between %s and %s. Is the bodyController module running?\n",local.c_str(),remote.c_str());

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
        printf("waiting for input \n");

        Bottle input;
        inputContextPort.read(input);



        Json::Reader reader;
        Json::Value attendedContext;

        printf("got input %s ----- \n",input.toString().c_str());
        string input_string = input.toString();
        prepareInput(input_string);

        reader.parse(input_string.c_str(),  attendedContext);

        std::cout << "Got context: " << '\n' << attendedContext.toStyledString() << '\n';

        Json::Value matchedSchema = engagement(attendedContext);

        //if(matchedSchema){

        for(Json::Value actionv: matchedSchema["actions"])
        {


            string action = actionv.asString();
            std::cout << "Dev-ER: Excecuting action '" << action << "'\n";

            if(action=="showInterestInV" || action=="showInterestInT" || action=="changeAttentionT"|| action=="changeAttentionV")  //mental actions
            {
                printf("mental action\n");
            }
            else   //bodily actions
            {
                //Send actions to body
                printf("bodily action\n");
                Bottle actioncmd;
                Bottle response;
                actioncmd.addString(action);
                output_port.write(actioncmd);
            }
int i = 0;
        while(i<200000000){i++;}
printf("done action2\n");
        }

       // } else { //cognitive impase
            //reflection();

        //}

        printf("now sending expectarions\n");

                Bottle expectativeBottle;

        if(!matchedSchema["expected"].empty()){
        Json::FastWriter fastWriter;
std::string expected = fastWriter.write(matchedSchema["expected"]);
        std::cout << "Writing out expectation: " << '\n' << expected << '\n';
            expectativeBottle.addString(expected);
        }else{
        std::cout << "Writing out expectation: {}"<< '\n';
        expectativeBottle.addString("{}");

        }




        outputExpectationPort.write(expectativeBottle);

        //wait for expectations request
        //string action ="mv";

    }


    virtual void threadRelease()
    {
        // we require an immediate stop
        // before closing the left_arm for safety reason

        printf("Stoping iDevER...\n");

        if(accommodated) //save the knowled base in file system
        {

        }
    }

    /**
    * Searches among the learned schemas for the best match for the current context.
    *
    * @param context to match against coded as a json document
    * @return the matched schema coded as a json document
    */

    Json::Value engagement(Json::Value context)
    {

        // std::cout << "context"<<context.toStyledString()<<'\n';
        Json::Value matchedSchema;
        float maxMatch = 0;
        int maxSpecificity = 0;
        /*if(allowPartialMatch) // if partial match is allowed
        {

        }
        else   //try a 100% matching
        {*/
            printf("Engagement: searching a match\n");
            // std::cout << "KB"<<kb["schemes"].toStyledString()<<'\n';
            for(Json::Value& schema: kb["schemes"])
            {
                //printf("matching %s\n",schema["id"].asString().c_str());

                float matchper = match(context,schema["context"]);
                int specificity = schema["context"][0].size()+schema["context"][0].size();

                std::cout << "schema [ " << schema["id"] <<"% ] matches "<< matchper <<"%\n";
if(matchper<100 && allowPartialMatch && schema["equilibrated"]=="-1"){
printf("Skiping non equilibrated schema [%s]",schema["id"].asString().c_str());
    continue;
}
                if(matchper==100 || allowPartialMatch){
                if(matchper>maxMatch || (matchper==maxMatch && specificity>maxSpecificity))
                {//priority to most specific schema
                    maxMatch = matchper;
                    maxSpecificity = specificity;
                    matchedSchema = schema;
                }

               }

            }

        //}

        std::cout << "Engagement: matched schema [ " << maxMatch <<"% ]"<< matchedSchema.toStyledString() <<'\n';
        return matchedSchema;
    }

    /**
    * Compares two contexts
    *
    * @param contexts to be compared, the first will be compared agains the second
    * @return matching percentaje. A 100 return value means a perfect match
    */


    float match(Json::Value context1,Json::Value context2)
    {
        //printf("matching...\n");
        float match = 0;

        if(context2[1].size()==0)  // if tactile context is empty then it matches anything
        {
            match+=50.0;
        }
        else
        {
            Json::Value::Members contenxtMembers = context1[1].getMemberNames();
            int membersSize = contenxtMembers.size();

            for(string memberName: contenxtMembers)
            {
                //std::cout << "chekin [ " << memberName <<" ]"<<'\n';
                if(context2[1][memberName].empty() || context1[1][memberName]==context2[1][memberName])
                {
                    match+=50.0/membersSize;
                }
            }

            contenxtMembers = context2[1].getMemberNames();
            membersSize = contenxtMembers.size();
            //std::cout << "camate [  ]"<<'\n';
            for(string memberName: contenxtMembers)
            {
                //std::cout << "chekin [ " << memberName <<" ]"<<'\n';
                if(context1[1][memberName].empty())
                {
                    match-=50.0/membersSize;
                }
            }
        }

        if(context2[0].size()==0)  // if visual context is empty then it matches anything
        {
            match+=50.0;
        }
        else
        {
            //printf("cisual contest not empty\n");

            Json::Value::Members contenxtMembers = context2[0].getMemberNames();
            int membersSize = contenxtMembers.size();

            //printf("Miembros => %s",contenxtMembers.toStyledString())
            /*std::cout << "size "<<membersSize<<'\n';
float r = (50.0/membersSize);
printf("rate %f\n",r);*/

            //std::cout << "camate [  ]"<<'\n';
            for(string memberName: contenxtMembers)
            {
                //printf( " match [ %f ]\n",match);
                if(!context1[0][memberName].empty() && context1[0][memberName]==context2[0][memberName])
                {
                    match+=50.0/membersSize;
                }
            }
/*
            contenxtMembers = context2[0].getMemberNames();
            membersSize = contenxtMembers.size();
            //std::cout << "camate [  ]"<<'\n';
            for(string memberName: contenxtMembers)
            {
                //std::cout << "chekin [ " << memberName <<" ]"<<'\n';
                if(context1[0][memberName].empty())
                {
                    match-=50.0/membersSize;
                }
            }*/

        }
        return match;

    }

    void test()
    {
        Json::Reader reader;
        Json::Value context;
        reader.parse("[{\"hap\":1,\"color\":\"red\",\"size\":\"0.3\"}, {}]",  context);

        Json::Value matchedSchema = engagement(context);

    }

    bool loadKB(string kb_file)
    {
        // Load jacub's knowledge base (schemes)
        //std::ifstream ifs { R"("++")" };
        std::ifstream ifs;
        ifs.open(kb_file);
        //std::ifstream ifs("("+kb_file+")" );
        if ( !ifs.is_open() )
        {
            std::cerr << "Could not load knokledge base "<<kb_file<<"\n";
            return EXIT_FAILURE;
        }

        Json::Reader reader;
        //Json::Value kb; // array of schemas
        reader.parse(ifs, kb);

        //std::cout << kb["schemes"].toStyledString() << '\n';

        // toStyledString	(		 )
        totalSchemes = kb["schemes"].size();

        std::cout << totalSchemes << " schemas loaded" << '\n';
    }
};



class DevER: public RFModule
{
protected:
    DevERThread *devERthr;

public:
    virtual bool configure(ResourceFinder &rf)
    {

        string robotName = rf.check("robot",Value("jacub")).asString();
        //string robotName = rf.find("robot").asString();
        printf("Encontró nombre de robot: %s\n",robotName.c_str());

        //Time::turboBoost();
        devERthr=new DevERThread(robotName, CTRL_THREAD_PER);
        if (!devERthr->start())
        {
            delete devERthr;
            return false;
        }

        devERthr->loadKB(rf.check("kb_file",Value("../../schemas/kb.json")).toString());

        return true;
    }

    virtual bool close()
    {
        devERthr->stop();
        delete devERthr;

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


    DevER idever;

    ResourceFinder rf;
    rf.setVerbose(); //logs searched directories
    rf.setDefaultConfigFile("config.ini"); //specifies a default configuration file
    rf.configure(argc,argv);
//idever.configure(rf);

//string robotName = rf.check("robot",Value("jacub")).asString();

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

    idever.runModule(rf);
    //return Network::connect("/perception/context/out","/memory/in");
    // return 0;
}

