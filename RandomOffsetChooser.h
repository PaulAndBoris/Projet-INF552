//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#ifndef GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H
#define GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H

#include "OffsetChooser.h"

class RandomOffsetChooser : public OffsetChooser{

private:
    bool checkOffset(const int offX, const int offY) const;

public:
    RandomOffsetChooser(const Image<Vec3b> *patch, const Image<uchar> *outputMask);

    virtual Point getNewOffset(bool *foundMask) const;

};


#endif //GRAPHCUT_TEXTURES_RANDOMOFFSETCHOOSER_H
