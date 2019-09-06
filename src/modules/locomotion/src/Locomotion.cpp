// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-
//
// A tutorial on how to use the Cartesian Interface to control a limb
// 5.251 25.727 5.850 83.259 64.800 29.700 0.000 59.000 20.000 20.000 20.000 10.000 10.000 10.000 10.000 10.000
//
// Author: Miguel Estrada - <luism@unam.mx>

#include <cstdio>
#include <cmath>

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/Time.h>
#include <yarp/sig/Vector.h>
#include <yarp/math/Math.h>

#include <yarp/dev/ControlBoardInterfaces.h>
#include <yarp/dev/CartesianControl.h>
#include <yarp/dev/PolyDriver.h>
#include <yarp/dev/IVelocityControl.h>
#include <jsoncpp/json/json.h>

#include <vector>
#include <jutils.h>

#include <fstream>
#include <iostream>

#define CTRL_THREAD_PER     0.02    // [s]
#define PRINT_STATUS_PER    1.0     // [s]
#define MAX_TORSO_PITCH     30.0    // [deg]

using namespace std;
using namespace yarp::os;
using namespace yarp::dev;
using namespace yarp::sig;
using namespace yarp::math;

class LocomotionModule: public RFModule {
protected:
	//ActionThread *actionThr;

	string robotName;
	ICartesianControl *left_arm_ctrl;
	IPositionControl *head_ctrl;
	PolyDriver head;
	PolyDriver left_arm;
	PolyDriver left_hand;
	IPositionControl *left_hand_ctrl;

	Port actionsInputPort;
	BufferedPort<Bottle> doneOutputPort;
	Port leftHandStateOutputPort;
	Vector xd;
	Vector od;
	bool moved;

	int startup_context_id;

	double t;
	double t0;
	double t1;
	int axe0_pos;
	int axe1_pos;
	int axe2_pos;
	int deltaAngle;
	//int clock;
	float xhorigin,yhorigin, zhorigin,deltaCoord;
	int cycles;
	Json::Reader jsonReader;
	string leftHandState;
	int closed;
//	int

public:
	/*Locomotion(){
		leftHandState = "open";
	}*/
	virtual bool configure(ResourceFinder &rf) {

		//Time::turboBoost();
		leftHandState = "open";
		closed = 0;
		string robotName = rf.check("robot", Value("jacub")).asString();

		 deltaAngle = rf.check("delta_angle", Value("10")).asInt();
		 deltaCoord = rf.check("delta_coord", Value("0.05")).asFloat32();

		/* actionThr=new BodyCtrlThread(robotName,deltaAngle, CTRL_THREAD_PER);
		 if (!actionThr->start())
		 {
		 delete actionThr;
		 return false;
		 }*/
		this->robotName = robotName;
		this->deltaAngle = 10;
		//clock = 0;
		/* cartesianEventParameters.type="motion-ongoing";
		 cartesianEventParameters.motionOngoingCheckPoint=0.2;*/

		string inPortName = "/" + robotName + "/locomotion/actions:i";
		if (!actionsInputPort.open(inPortName)) {
			yError(
					"Failed creating input port for locomotion module (%s)",inPortName.c_str());
			return false;
		}

		string outPortName = "/" + robotName + "/locomotion/done:o";
				if (!doneOutputPort.open(outPortName)) {
					yError(
							"Failed creating continue command output port for locomotion module (%s)",outPortName.c_str());
					return false;
				}

		moved = false;
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
		axe0_pos = 0;
		axe1_pos = 0;
		axe2_pos = 0;

		string leftArmCtrPortName = "/" + robotName + "/left_arm";
		string remote_port = "/icubSim/cartesianController/left_arm";

		Property options("(device cartesiancontrollerclient)");
		options.put("remote", remote_port);
		options.put("local", leftArmCtrPortName);
		if (!left_arm.open(options)) {
			yError(
					"Couldn't get cartesian left_arm controller (%s). Is the cartesian server running?\n",remote_port.c_str());
			//return false;
		}

		string leftHandCtrPortName = "/" + robotName + "/left_hand";
		options.put("device", "remote_controlboard");
		options.put("remote", "/icubSim/left_arm");
		options.put("local", leftHandCtrPortName);

		if (!left_hand.open(options)) {
			yError(
					" Couldn't get left_arm controller (%s). Is the iCub_SIM running?\n",leftHandCtrPortName.c_str());
			//return false;
		}

		//yDebug("returned -1\n");
		left_arm.view(left_arm_ctrl);
		left_hand.view(left_hand_ctrl);
		// latch the controller context in order to preserve
		// it after closing the module
		// the context contains the dofs status, the tracking mode,
		// the resting positions, the limits and so on.
		left_arm_ctrl->storeContext(&startup_context_id);
		// set trajectory time
		left_arm_ctrl->setTrajTime(1.0);
		// get the torso dofs
		Vector newDof, curDof;
		left_arm_ctrl->getDOF(curDof);
		newDof = curDof;

		// enable the torso yaw and pitch
		// disable the torso roll
		newDof[0] = 1;
		newDof[1] = 0;
		newDof[2] = 1;

		// send the request for dofs reconfiguration
		left_arm_ctrl->setDOF(newDof, curDof);

		// impose some restriction on the torso pitch
		limitTorsoPitch();

		// print out some info about the controller
		//Bottle info;
		//left_arm_ctrl->getInfo(info);
		//fprintf(stdout,"left arm info = %s\n",info.toString().c_str());

		// register the event, attaching the callback
		// left_arm_ctrl->registerEvent(*this);

		xd.resize(3);
		od.resize(4);

		string headCtrPortName = "/" + robotName + "/head";
		options.put("device", "remote_controlboard");
		options.put("local", headCtrPortName);
		options.put("remote", "/icubSim/head");
		//yDebug("returned0 \n");
		if (!head.open(options)) {
			yError("Couldn't get head controller. Is the iCub_SIM running?");
			return false;
		}


		outPortName = "/" + robotName + "/locomotion/leftHandState:o";
				if (!leftHandStateOutputPort.open(outPortName)) {
					yError(
							"Failed creating continue command output port for left hand state (%s)",outPortName.c_str());
					return false;
				}

		//yDebug("returned\n");

		head.view(head_ctrl);
		cycles = 0;

		xhorigin = -0.35;
		yhorigin = -0.2;
		zhorigin = -0.005;//-0.19//origin, zhorigin;
		//yDebug("returned\n");
		setInitialLeftHandPos();
		//setInitialHeadPos();

		return true;
	}

	virtual bool close() {
//close ports

		left_arm_ctrl->stopControl();
//head_ctrl->stopControl();

		// it's a good rule to restore the controller
		// context as it was before opening the module
		left_arm_ctrl->restoreContext(startup_context_id);

		left_arm.close();

		return true;
	}

	virtual double getPeriod() {
		return 1.0;
	}
	virtual bool updateModule() {
		//moveHeadDown(60);
		//
		yDebug(" Locomotion: cycle %d",++cycles);

		yDebug(" Locomotion: waiting for actions \n");
		Bottle input;
		actionsInputPort.read(input);
		//if (input!=NULL) {
		//contextInputPort.read(input);
		string input_string = input.toString();
		prepareInput(input_string);
		Json::Value commandedActions;
		jsonReader.parse(input_string.c_str(), commandedActions);
		yDebug(" Locomotion: Got %s\n",input_string.c_str());

		//string action = input.toString();
		bool done = false;

		/*vector<std::string> actions { "headUp", "headDown", "headLeft",
				"headRight", "headLeftUp", "headLeftDown", "headRightUp",
				"headRightDown", "handUp", "handDown", "handLeft", "handRight",
				"handBackward", "handForward", "closeHand", "openHand" };*/
		vector<std::string> actions { "headUp", "headDown", "headLeft",
						"headRight", "headLeftUp", "headLeftDown", "headRightUp",
						"headRightDown"};


		for (Json::Value& action : commandedActions) {
			yDebug(" Locomotion: ACTION %s\n",action.toStyledString().c_str());
			if (action == "random") {
				srand(time(NULL));
				int randomi = rand() % actions.size();
				yDebug(" Locomotion: Random action");
				/*for(int i = 0; i < action.size(); i++)
				 {*/
				float random_index = rand() % actions.size();
				yDebug(" Locomotion: random seed=%f",random_index);
				action = actions[random_index];
				//}
			}

			if (action == "headUp") {
				yDebug(" Locomotion: Moving head up ...\n");
				done = moveHead(deltaAngle, 0, 0);
			} else if (action == "headDown") {
				yDebug(" Locomotion: Moving head down ...\n");
				done = moveHead(-deltaAngle, 0, 0);
			} else if (action == "headLeft") {
				yDebug(" Locomotion: Moving head left ...\n");
				done = moveHead(0, 0, deltaAngle);
			} else if (action == "headRight") {
				yDebug(" Locomotion: Moving head right ...\n");
				done = moveHead(0, 0, -deltaAngle);
			} else if (action == "headLeftUp") {
				yDebug(" Locomotion: Moving head left and up ...\n");
				done = moveHead(deltaAngle, 0, deltaAngle);
				/*if(done)
				 {
				 yDebug("Moving head up ...\n");
				 done = moveHeadUp(deltaAngle);
				 }*/
			}

			else if (action == "headLeftDown") {
				yDebug(" Locomotion: Moving head left and down ...\n");
				done = moveHead(-deltaAngle, 0, deltaAngle);
				/*if(moveHeadLeft(deltaAngle))
				 {
				 yDebug("Moving head down ...\n");
				 done = moveHeadDown(deltaAngle);
				 }*/
			} else if (action == "headRightUp") {
				yDebug(" Locomotion: Moving head right and up ...\n");
				done = moveHead(deltaAngle, 0, -1 * deltaAngle);

				/*if(moveHeadRight(deltaAngle))
				 {
				 yDebug("Moving head up ...\n");
				 done = moveHeadUp(deltaAngle);
				 }**/
			} else if (action == "headRightDown") {
				yDebug(" Locomotion: Moving head right and down ...\n");
				done = moveHead(-deltaAngle, 0, -deltaAngle);
			} else if (action == "handRight") {
				yDebug(" Locomotion: Moving hand right ...\n");
				done = moveLeftArm(xhorigin,yhorigin+deltaCoord,zhorigin, 0.0, 0.0, 4.0);
												if(done) yhorigin+=deltaCoord;
												done = moveHead(0, 0, deltaAngle);

			} else if (action == "handLeft") {
				yDebug(" Locomotion: Moving hand left ...\n");
				done = moveLeftArm(xhorigin,yhorigin-deltaCoord,zhorigin, 0.0, 0.0, 4.0);
												if(done) yhorigin-=deltaCoord;
			} else if (action == "handUp") {
				yDebug(" Locomotion: Moving hand up %f ...\n",deltaCoord);
				done = moveLeftArm(xhorigin,yhorigin,zhorigin+2*deltaCoord, 0.0, 0.0, 4.0);
								if(done) zhorigin+=2*deltaCoord;
			} else if (action == "handDown") {
				yDebug(" Locomotion: Moving hand down ...\n");
				done = moveLeftArm(xhorigin,yhorigin,zhorigin-deltaCoord, 0.0, 0.0, 4.0);
				if(done) zhorigin-=deltaCoord;
			} else if (action == "handBackward") {
				yDebug(" Locomotion: Moving hand backwards ...\n");
				done = moveLeftArm(xhorigin+deltaCoord,yhorigin,zhorigin, 0.0, 0.0, 4.0);
								if(done)xhorigin+=deltaCoord;

			} else if (action == "handForward") {
			} else if (action == "closeHand" && leftHandState!="closed") {
				yDebug(" Locomotion: Closing left hand ...\n");
				done = closeHand();
				//if(done){
					leftHandState = "closed";
				//}
				done = moveLeftArm(xhorigin+deltaCoord,yhorigin,zhorigin+deltaCoord, 0.0, 0.0, 4.0);
				if(done)zhorigin+=deltaCoord;
				//closeHand();
			} else if (action == "openHand") {
				yDebug(" Locomotion: Opening left hand ...\n");
				done = openHand();
				if(done){
									leftHandState = "open";
								}
			} else {
				yWarning("Unknown action :'%s'\n",action.toStyledString().c_str());
			}
			//yDebug(" Locomotion: Action performed \n";
		}

		yDebug(" Locomotion: writing out left hand state [%s]",leftHandState.c_str());
		yDebug(" Locomotion: closed %d",closed);
		if(leftHandState=="closed") closed++;

		if(closed>5 && openHand()){
			closed = 0;
		}
yDebug(" Locomotion: hand state %s",leftHandState.c_str());
		Bottle output;
		output.addString(leftHandState);
		leftHandStateOutputPort.write(output);

		yDebug(" Locomotion: Actions performed, writing out done \n");

		 output = doneOutputPort.prepare();
		output.addString(done ? "true" : "false");
		doneOutputPort.write();
		//doneOutputPort.wr
		return true;
	}

private:

	void setInitialHeadPos(){
		yInfo(" Setting initial head position for %s\n robot",robotName.c_str());

		//moveHead(-60, 0, -15);
		moveHead(-60, 0, 25);
		//moveHead(int angle0, int angle1, int angle2) {
	}

	void setInitialLeftHandPos(){
		yInfo(" Setting initial left hand position for %s\n robot",robotName.c_str());

		moveLeftArm(xhorigin,yhorigin,zhorigin,0.0,0.0,4.0);
	}




	bool moveHead(int angle0, int angle1, int angle2) {
		// yDebug("DENTRO %d %d %d\n",angle0, angle1, angle2);
		IEncoders *enc;
		IVelocityControl *vel;

		head.view(enc);
		head.view(vel);
		int njoints = 0;
		enc->getAxes(&njoints);
		/*Vector v;
		 Vector command;
		 command.resize(njoints);
		 yDebug("joints %s",njoints);
		 v.resize(njoints);*/

		Vector command;
		command.resize(njoints);
		enc->getEncoders(command.data());

		//yDebug("\ncommando => %s\n\n\n",command.toString().c_str());

		command[0] = command[0] + angle0;
		command[1] = command[1] + angle1;
		command[2] = command[2] + angle2;

		return head_ctrl->positionMove(command.data());

		/* axe0_pos = 0;
		 axe1_pos=0;
		 axe2_pos=0;*/
		//return true;
	}

	/**
	 * Moves the left arm of the robot as indicated by the given coordenates and orientation
	 * @param[in] x (x>0,0,0) points behind the robot
	 * @param[in] y (0,y>0,0) points to the right side of the robot
	 * @param[in] z (0,0,z>0) is the normal vector of the floor pointing upwards
	 * @return true y the movement succeded
	 */
	bool moveLeftArm(double x, double y, double z, double ox, double oy,
			double oz) {

		// translational target part: a circular trajectory
		// in the yz plane centered in [-0.3,-0.1,0.1] with radius=0.1 m
		// and frequency 0.1 Hz
		//-0.55387995 0.35 -0.09
		//-0.35 -0.09 0.55387995
		/*xd[0]=-0.35;
		 xd[1]=-0.09;
		 xd[2]=+0.55387995;*/
		/*xd[0]=-0.35;
		 xd[1]=-0.1;
		 xd[2]=+0.002;*/
		xd[0] = x;
		xd[1] = y;
		xd[2] = z;

		// we keep the orientation of the left arm constant:
		// we want the middle finger to point forward (end-effector x-axis)
		// with the palm turned down (end-effector y-axis points leftward);
		// to achieve that it is enough to rotate the root frame of pi around z-axis
		od[0] = ox;
		od[1] = oy;
		od[2] = oz;
		od[3] = -M_PI;

		left_arm_ctrl->goToPose(xd, od);
		bool done = left_arm_ctrl->waitMotionDone(0.1, 4.0);
		if (!done) {
			yDebug("Locomotion: Left arm is taking to long to complete the movement\n");
		}
		return done;
	}

	bool moveWrist() {
		IEncoders *enc;
		IVelocityControl *vel;

		left_hand.view(enc);
		left_hand.view(vel);

		int njoints = 0;
		enc->getAxes(&njoints);
		//Vector v;
		Vector command;
		command.resize(njoints);
		enc->getEncoders(command.data()); //get the current position

		//command.resize(njoints);

		//v.resize(njoints);
		//New positions for the fingers
		//15.0 80.0   0.0   0.0  70.0   0.0  70.0   0.0  60.0
		//0.0   80.0   10.0   40.0   30.0   60.0   20.0   50.0   150.0
		/*command[7]=0.0;
		 command[8]= 80.0;
		 command[9]=12.0;
		 command[10]=18.0;
		 command[11]=0.0;  //command[11]=27.0;
		 command[12]=90.0; //command[12]=50.0;
		 command[13]=20.0;
		 command[14]=50.0;
		 command[15]=135.0;*/
		/*command[7]=15.0;
		 command[8]= 80.0;
		 command[9]=0.0;
		 command[10]=0.0;
		 command[11]=70.0;  //command[11]=27.0;
		 command[12]=0.0; //command[12]=50.0;
		 command[13]=70.0;
		 command[14]=0.0;
		 command[15]=60.0;*/
		/*command[5]=-70.0;
		 command[7]=0.0; //juntar dedos - = mas abiertos
		 command[8]= 88.0; //pulgar hacia palma
		 command[9]=0.0; //pulgar despegar
		 command[10]=54.0; //doblar pulgar
		 command[11]=56.7;  //command[11]=27.0; //doblar indicie hacia palma
		 command[12]=104.0; //command[12]=50.0; //doblar indice +=más doblado
		 command[13]=70.0; //dedo medio doblado +=mas doblado
		 command[14]=61.0; //dedo medio doblas
		 command[15]=151.0; //meñique doblar +=mas doblado*/
		command[5] = 0.0;
		command[6] = 22.2;
		command[7] = 58.8;
		command[8] = 20.0;
		command[9] = 19.8;
		command[10] = 19.8;
		command[11] = 9.9;
		command[12] = 10.8;
		command[13] = 9.9;
		command[14] = 10.8;
		command[15] = 10.8;
		//command[7]=0.0; //juntar dedos - = mas abiertos
		//command[8]= 88.0; //pulgar hacia palma
		// command[9]=0.0; //pulgar despegar
		//command[10]=54.0; //doblar pulgar
		//command[11]=56.7;  //command[11]=27.0; //doblar indicie hacia palma
		//command[12]=104.0; //command[12]=50.0; //doblar indice +=más doblado
		//command[13]=70.0; //dedo medio doblado +=mas doblado
		//command[14]=61.0; //dedo medio doblas
		//command[15]=151.0; //meñique doblar +=mas doblado*/
		//articulación 5 muñeca arroba abajo
		/* command[7]=58.8; //juntar dedos - = mas abiertos
		 command[8]= 80.8; //pulgar hacia palma
		 command[9]=0.0; //pulgar despegar
		 command[10]=104.4; //doblar pulgar
		 command[11]=46.8;  //command[11]=27.0; //doblar indicie hacia palma
		 command[12]=86.4; //command[12]=50.0; //doblar indice +=más doblado
		 command[13]=50.0; //dedo medio doblado +=mas doblado
		 command[14]=81.0; //dedo medio doblas
		 command[15]=137.0; //meñique doblar +=mas doblado*/
		yDebug(" Locomotion: commando => %s\n\n\n", command.toString().c_str());

		left_hand_ctrl->positionMove(command.data());
		yDebug(" Locomotion: wrist moving upwards...\n");

		bool done = left_arm_ctrl->waitMotionDone(0.1, 4.0);
		if (!done) {
			yDebug(" Locomotion: Left arm is taking to long to complete the movement\n");
		}
		yDebug(" Locomotion: Done wrist %d\n", done);
		return done;

	}

	bool closeHand() {
		IEncoders *enc;
		IVelocityControl *vel;

		left_hand.view(enc);
		head.view(vel);

		int njoints = 0;
		enc->getAxes(&njoints);
		Vector v;
		Vector command;
		command.resize(njoints);
		enc->getEncoders(command.data()); //get the current position

		command.resize(njoints);

		v.resize(njoints);
		//New positions for the fingers
		//15.0 80.0   0.0   0.0  70.0   0.0  70.0   0.0  60.0
		//0.0   80.0   10.0   40.0   30.0   60.0   20.0   50.0   150.0
		/*command[7]=0.0;
		 command[8]= 80.0;
		 command[9]=12.0;
		 command[10]=18.0;
		 command[11]=0.0;  //command[11]=27.0;
		 command[12]=90.0; //command[12]=50.0;
		 command[13]=20.0;
		 command[14]=50.0;
		 command[15]=135.0;*/
		/*command[7]=15.0;
		 command[8]= 80.0;
		 command[9]=0.0;
		 command[10]=0.0;
		 command[11]=70.0;  //command[11]=27.0;
		 command[12]=0.0; //command[12]=50.0;
		 command[13]=70.0;
		 command[14]=0.0;
		 command[15]=60.0;i/
		 */
		yDebug(" Locomotion: commandobe> %s\n\n\n", command.toString().c_str());
		command[5] = -37.0;
		command[7] = 0.0; //juntar dedos - = mas abiertos
		command[8] = 88.0; //pulgar hacia palma
		command[9] = 0.0; //pulgar despegar
		command[10] = 54.0; //doblar pulgar
		command[11] = 56.7;  //command[11]=27.0; //doblar indicie hacia palma
		command[12] = 104.0; //command[12]=50.0; //doblar indice +=más doblado
		command[13] = 70.0; //dedo medio doblado +=mas doblado
		command[14] = 61.0; //dedo medio doblas
		command[15] = 151.0; //meñique doblar +=mas doblado*/
		//articulación 5 muñeca arroba abajo
		/* command[7]=58.8; //juntar dedos - = mas abiertos
		 command[8]= 80.8; //pulgar hacia palma
		 command[9]=0.0; //pulgar despegar
		 command[10]=104.4; //doblar pulgar
		 command[11]=46.8;  //command[11]=27.0; //doblar indicie hacia palma
		 command[12]=86.4; //command[12]=50.0; //doblar indice +=más doblado
		 command[13]=50.0; //dedo medio doblado +=mas doblado
		 command[14]=81.0; //dedo medio doblas
		 command[15]=137.0; //meñique doblar +=mas doblado*/

		return left_hand_ctrl->positionMove(command.data());
		//yDebug("closing hand...%d\n",done);
		//return done;

	}

	bool openHand() {
		IEncoders *enc;
		IVelocityControl *vel;

		left_hand.view(enc);
		head.view(vel);

		int njoints = 0;
		enc->getAxes(&njoints);
		Vector v;
		Vector command;
		command.resize(njoints);
		enc->getEncoders(command.data()); //get the current position
		command.resize(njoints);
		v.resize(njoints);

		//0.000723	 79.997541	 0.000561	 49.999764	-0.000103	 30.000009	 0.000003	 58.999975	 19.999992	 19.999991	 19.999992	 9.999996	 9.999996	 9.999996	 9.999996	 9.999997
		command[0] = 0.000723;
		command[1] = 79.997541;
		command[2] = 0.000561;
		command[3] = 49.999764;
		command[4] = -0.000103;
		command[5] = 30.000009;
		command[6] = 0.000003; //juntar dedos - = mas abiertos
		command[7] = 58.999975; //pulgar hacia palma
		command[8] = 19.999992; //pulgar despegar
		command[9] = 19.999991; //doblar pulgar
		command[10] = 19.999992; //command[11]=27.0; //doblar indicie hacia palma
		command[11] = 9.999996; //command[12]=50.0; //doblar indice +=más doblado
		command[12] = 9.999996; //dedo medio doblado +=mas doblado
		command[13] = 9.999996; //dedo medio doblas
		command[14] = 9.999996; //meñique doblar +=mas doblado*/
		command[15] = 9.999997;

		yDebug("COMMAND");
		bool done = left_hand_ctrl->positionMove(command.data());
 return true;
	}

	void limitTorsoPitch() {
		int axis = 0; // pitch joint
		double min, max;

		// sometimes it may be helpful to reduce
		// the range of variability of the joints;
		// for example here we don't want the torso
		// to lean out more than 30 degrees forward

		// we keep the lower limit
		left_arm_ctrl->getLimits(axis, &min, &max);
		left_arm_ctrl->setLimits(axis, min, MAX_TORSO_PITCH);
	}

	void printStatus() {
		if (t - t1 >= PRINT_STATUS_PER) {
			Vector x, o, xdhat, odhat, qdhat;

			// we get the current arm pose in the
			// operational space
			left_arm_ctrl->getPose(x, o);

			// we get the final destination of the arm
			// as found by the solver: it differs a bit
			// from the desired pose according to the tolerances
			left_arm_ctrl->getDesired(xdhat, odhat, qdhat);

			double e_x = norm(xdhat - x);
			double e_o = norm(odhat - o);

			fprintf(stdout, "+++++++++\n");
			fprintf(stdout, "xd          [m] = %s\n", xd.toString().c_str());
			fprintf(stdout, "xdhat       [m] = %s\n", xdhat.toString().c_str());
			fprintf(stdout, "x           [m] = %s\n", x.toString().c_str());
			fprintf(stdout, "od        [rad] = %s\n", od.toString().c_str());
			fprintf(stdout, "odhat     [rad] = %s\n", odhat.toString().c_str());
			fprintf(stdout, "o         [rad] = %s\n", o.toString().c_str());
			fprintf(stdout, "norm(e_x)   [m] = %g\n", e_x);
			fprintf(stdout, "norm(e_o) [rad] = %g\n", e_o);
			fprintf(stdout, "---------\n\n");

			t1 = t;
		}
	}
};

