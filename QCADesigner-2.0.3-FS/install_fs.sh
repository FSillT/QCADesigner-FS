#!/bin/bash

echo "Installation starts of QCADesign 2.0.3-FS" 

./autogen.sh
./configure
make
rm -rf src
mv srcfs src
./configure
make
mkdir bin
cp src/QCADesigner bin/.

echo "Installation is done" 
echo "start ./bin/QCADesigner" 
