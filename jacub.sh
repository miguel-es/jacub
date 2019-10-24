#!/bin/bash
# Jacub startup script

if [ ! -d ~/.config/yarp/contexts ]; then
  ln -s contexts/ ~/.config/yarp
fi

if [ ! -d log ]; then
  mkdir log
fi

echo "Compiling Jacub source code..."
sudo cmake src/modules/world
sudo make --directory=src/modules/world install

sudo cmake src/modules/memory
sudo make --directory=src/modules/memory install

sudo cmake src/modules/perception
sudo make --directory=src/modules/perception install
sudo cmake src/modules/attention
sudo make --directory=src/modules/attention install
sudo cmake src/modules/emotion
sudo make --directory=src/modules/emotion install
sudo cmake src/modules/DevER
sudo make --directory=src/modules/DevER install
sudo cmake src/modules/locomotion
sudo make --directory=src/modules/locomotion install
sudo cmake .
sudo make install 



echo "Starting modules..."

pid=$(pidof iCubSkinGui yarpview world perception memory  attention emotion DevER locomotion iCub_SIM yarprobotinterface iKinCartesianSolver motionCUT)
sudo kill -9 $pid

#echo "Starting yarpserver..."
#nohup yarpserver > log/yarpserver.log 2>&1 &
echo "Starting iCub simulator..."

nohup iCub_SIM > log/iCubSim.log 2>&1 &
nohup motionCUT --coverXratio 1 --coverYratio 1 --nodesStep 3 --verbosity on --winSize 3 --adjNodesThres 5 --blobMinSizeThres 1 --framesPersistence 0> log/motionCUT.log 2>1 &
yarpview --x 1100 --y 0 > log/yarpview.log 2>&1 &
#nohup yarpview --name /view/motion:i> log/yarpview.log 2>&1 &
sleep 3
nohup yarprobotinterface --context simCartesianControl > log/yarprobotinterface.log 2>&1 &
nohup iKinCartesianSolver --context simCartesianControl --part left_arm > log/iKinCartesianSolver.log 2>&1 &

sleep 3
echo "Starting memory module..."
nohup memory --context jacub > log/memory.log 2>&1 &
echo "Starting perception module..."
nohup perception --context jacub > log/perception.log 2>&1 &
echo "Starting attention module..."
nohup attention --context jacub > log/attention.log 2>&1 &
echo "Starting emotion module..."
nohup emotion --context jacub > log/emotion.log 2>&1 &
echo "Starting DevER module..."
nohup DevER --context jacub > log/DevER.log 2>&1 &
echo "Starting locomotion module..."
nohup locomotion --context jacub > log/locomotion.log 2>&1 &

sleep 7
echo "Initializing world..."
nohup world >log/world.log 2>&1 &

iCubSkinGui --useCalibration> log/ikinGui.log 2>&1 &
echo 'waiting for world to be up...'
yarp wait /world/continue:i
yarp connect /jacub/locomotion/done:o /world/continue:i
yarp connect /icubSim/cam/left /view/img:i
#yarp connect /motionCUT/img:o /view/motion:i
echo 'waiting for iCubSkinGui to be alive...'
yarp wait /skinGui/left_hand:i
yarp connect /icubSim/skin/left_hand_comp /skinGui/left_hand:i
yarp connect /icubSim/skin/left_hand_comp /skinGui/left_hand:i

mind

echo "Jacub has started. Logs for cognitive processes are available in log/ folder"


