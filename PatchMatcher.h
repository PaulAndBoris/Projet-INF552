//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#ifndef GRAPHCUT_TEXTURES_PATCHMATCHER_H
#define GRAPHCUT_TEXTURES_PATCHMATCHER_H

#include "OffsetChooser.h"
#include "Patcher.h"

class PatchMatcher : public OffsetChooser {

private:
    const Image<Vec3b> *patch;
    const Image<uchar> *outputMask;
    const Image<Vec3b> *output;
    double factor;

    Point nextOffset;

public:
    PatchMatcher(const Image<Vec3b> *patch, const Image<Vec3b> *output, const Image<uchar> *outputMask);

    virtual Point getNewOffset(Image<Vec3b> *newPatch, bool *foundMask);

};


#endif //GRAPHCUT_TEXTURES_PATCHMATCHER_H
