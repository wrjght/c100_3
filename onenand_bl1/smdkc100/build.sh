#!/bin/bash

echo 1. Make BL1 image
make all

echo 2. Padding checksum
./mkbl1

echo 3. Generate U-boot image: ../u-boot-onnand.bin
cat BL1.bin.padding ../u-boot.bin > ../u-boot-onenand.bin

