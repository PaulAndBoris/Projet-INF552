//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#ifndef GRAPHCUT_TEXTURES_PATCHER_H
#define GRAPHCUT_TEXTURES_PATCHER_H

#include "image.h"

class Patcher {

private:
    const Image<Vec3b> patch;
    Image<float> oldSeams;
    Image<Vec3b> output;

    RandomOffsetChooser rndOffsetChooser;

public:

    Patcher(const Mat &patch, size_t width, size_t height);
    const Mat& randomStep();
};


#endif //GRAPHCUT_TEXTURES_PATCHER_H
