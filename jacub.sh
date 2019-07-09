#!/bin/bash
# Jacub startup scrip

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



pid=$(pidof memory perception attention emotion DevER locomotion iCub_SIM yarprobotinterface iKinCartesianSolver)
kill -9 $pid

#nohup yarpserver&
nohup iCub_SIM&
sleep 3
nohup yarprobotinterface --context simCartesianControl > log/yarprobotinterface.log 2>&1 &
nohup iKinCartesianSolver --context simCartesianControl --part left_arm > log/iKinCartesianSolver.log 2>&1 &

sleep 3
world
nohup memory --context jacub > log/memory.log 2>&1 &
nohup perception --context jacub > log/perception.log 2>&1 &
nohup attention --context jacub > log/attention.log 2>&1 &
nohup emotion --context jacub > log/emotion.log 2>&1 &
nohup DevER --context jacub > log/DevER.log 2>&1 &
nohup locomotion --context jacub > log/locomotion.log 2>&1 &

sleep 3

mind

