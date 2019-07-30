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
sudo cmake src/modules/DevER
sudo make --directory=src/modules/locomotion install
sudo cmake .
sudo make install 



echo "Starting modules..."

pid=$(pidof world perception memory  attention  DevER locomotion iCub_SIM yarprobotinterface iKinCartesianSolver)
sudo kill -9 $pid

#echo "Starting yarpserver..."
#nohup yarpserver > log/yarpserver.log 2>&1 &
echo "Starting iCub simulator..."

nohup iCub_SIM > log/iCubSim.log 2>&1 &
sleep 3
nohup yarprobotinterface --context simCartesianControl > log/yarprobotinterface.log 2>&1 &
nohup iKinCartesianSolver --context simCartesianControl --part left_arm > log/iKinCartesianSolver.log 2>&1 &

sleep 3
echo "Initializing world..."
nohup world >log/world.log 2>&1 &
echo "Starting memory module..."
nohup memory --context jacub > log/memory.log 2>&1 &
echo "Starting perception module..."
nohup perception --context jacub > log/perception.log 2>&1 &
echo "Starting attention module..."
nohup attention --context jacub > log/attention.log 2>&1 &
echo "Starting emotion module..."
#nohup emotion --context jacub > log/emotion.log 2>&1 &
echo "Starting DevER module..."
nohup DevER --context jacub > log/DevER.log 2>&1 &
echo "Starting locomotion module..."
nohup locomotion --context jacub > log/locomotion.log 2>&1 &

sleep 5
mind

echo "Jacub has started. Logs for cognitive processes are available in log/ folder"


