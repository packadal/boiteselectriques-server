#!/bin/sh

# Run the server in an infinite loop

LD_LIBRARY_PATH=:/usr/local/lib/arm-linux-gnueabihf/ 
export LD_LIBRARY_PATH

while true; do ./be-server; done
