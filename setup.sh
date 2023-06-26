source /cvmfs/sft.cern.ch/lcg/views/LCG_102/x86_64-centos7-gcc11-opt/setup.sh
g++ server.cpp -lpthread -o server
g++ client.cpp -lpthread -o client
