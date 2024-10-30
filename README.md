# VamanaAlgorithm
Implementation of vamana algorithm to serve queries
 
# Install Google Test
sudo apt-get update
sudo apt-get install libgtest-dev cmake
 
# Build Google Test
cd /usr/src/gtest
sudo cmake .
sudo make
sudo cp lib/libgtest*.a /usr/lib/
 
# Build the project
make
 
# Run tests
make test
 
# Run the main program
make run
 
# Optionally, run the main program with arguments
make run ARGS="arg1 arg2"
