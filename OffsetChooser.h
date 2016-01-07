//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#ifndef GRAPHCUT_TEXTURES_OFFSETCHOOSER_H
#define GRAPHCUT_TEXTURES_OFFSETCHOOSER_H

#include "image.h"

class OffsetChooser {

protected:
    const Image<Vec3b> *patch;
    const Image<uchar> *outputMask;

    static void getBoundaries(const Image<uchar> &mask, int *pInt, int *pInt1, int *pInt2, int *pInt3);
    static int randRange(const int min, const int max);

public:
    OffsetChooser(const Image<Vec3b> *patch, const Image<uchar> *outputMask);
    virtual ~OffsetChooser();

    virtual Point getNewOffset(bool *foundMask) const = 0;
};


#endif //GRAPHCUT_TEXTURES_OFFSETCHOOSER_H
