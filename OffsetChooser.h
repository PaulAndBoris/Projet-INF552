//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#ifndef GRAPHCUT_TEXTURES_OFFSETCHOOSER_H
#define GRAPHCUT_TEXTURES_OFFSETCHOOSER_H

#include "image.h"

class OffsetChooser {

protected:
    static void getBoundaries(const Image<uchar> &mask, int *pInt, int *pInt1, int *pInt2, int *pInt3);
    static int randRange(const int min, const int max);

public:
    OffsetChooser();
    virtual ~OffsetChooser();

    virtual Point getNewOffset(Image<Vec3b> *newPatch, bool *foundMask) = 0;
};


#endif //GRAPHCUT_TEXTURES_OFFSETCHOOSER_H
