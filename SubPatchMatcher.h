//
// Created by Paul VANHAESEBROUCK on 07/01/2016.
//

#ifndef GRAPHCUT_TEXTURES_SUBPATCHMATCHER_H
#define GRAPHCUT_TEXTURES_SUBPATCHMATCHER_H


#include "OffsetChooser.h"
#include "Patcher.h"

class SubPatchMatcher : public OffsetChooser {

private:
    const Image<Vec3b> *patch;
    const Image<Vec3b> *output;
    const Image<uchar> *outputMask;
    const Patcher *patcher;

    Point nextOffset;
    const int windowWidth, windowHeight;

    Point computeWindowOffset() const;

public:
    SubPatchMatcher(const Image<Vec3b> *patch, const Image<Vec3b> *output, const Image<uchar> *outputMask, const Patcher *patcher);

    virtual Point getNewOffset(Image<Vec3b> &newPatch, bool *foundMask);

};


#endif //GRAPHCUT_TEXTURES_SUBPATCHMATCHER_H
