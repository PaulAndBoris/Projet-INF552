//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include "Patcher.h"

Patcher::Patcher(const Mat& Patch, size_t width, size_t height) : patch(patch), output(Mat::zeros(height, width, CV_8UC3)), oldSeams(Mat::zeros(height, width,CV_32F)){};

const Mat& Patcher::randomStep(){

};