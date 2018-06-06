#!/bin/bash

make clean TARGET=z1
make TARGET=z1
make app TARGET=z1
msp430-objcopy app.z1 -O ihex app.ihex
sudo make app.upload TARGET=z1

