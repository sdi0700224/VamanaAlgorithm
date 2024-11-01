# VamanaAlgorithm
This project implements the Vamana algorithm, designed for efficient k-nearest neighbor (k-NN) search in high-dimensional data spaces.

# Install Google Test
sudo apt-get update
sudo apt-get install libgtest-dev cmake
 
# Build Google Test (Optional)
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
