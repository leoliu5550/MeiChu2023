#!/usr/bin/env bash
pwd=`pwd`
echo "" > $pwd/Firmware/tflite/sample_imgs.h
cd ./Model/test_imgs
for file in *.bmp ; do
  echo "const static " | tr -d '\n' >> $pwd/Firmware/tflite/sample_imgs.h
  xxd -i $file >> $pwd/Firmware/tflite/sample_imgs.h
done
