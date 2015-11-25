//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#ifndef GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H
#define GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H

#include "image.h"

class RandomOffsetChooser {

private:
    const Image<Vec3b> *patch;
    const Image<uchar> *outputMask;

    void getBoundaries(const Image<uchar> &mask, int *pInt, int *pInt1, int *pInt2, int *pInt3) const;
    inline int randRange(const int min, const int max) const;
    bool checkOffset(const int offX, const int offY) const;

public:
    RandomOffsetChooser(const Mat *patch, const Mat *outputMask);

    Point const getNewOffset() const;

};


#endif //GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H
