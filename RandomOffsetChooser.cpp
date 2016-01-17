//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include "RandomOffsetChooser.h"

#define OVERLAP_WIDTH (1<<4)

RandomOffsetChooser::RandomOffsetChooser(const Image<Vec3b> *patch, const Image<uchar> *outputMask) :
        OffsetChooser(),
        patch(patch),
        outputMask(outputMask) {

}

// Returns a valid offset for the new inputPatch to apply
Point RandomOffsetChooser::getNewOffset(Image<Vec3b> *newPatch, bool *foundMask) {

    *newPatch = *patch;

    int minX, maxX, minY, maxY;
    getBoundaries(*outputMask, &minX, &maxX, &minY, &maxY);

    if (maxX < 0 || maxY < 0) {
        *foundMask = false;
        return Point(randRange(0, outputMask->width()  - patch->width()  + 1),
                     randRange(0, outputMask->height() - patch->height() + 1));
    } else {
        *foundMask = true;

        int offX, offY;
        do {
            offX = randRange(max(0, minX - patch->width()  + 1), min(maxX, outputMask->width()  - patch->width()  + 1));
            offY = randRange(max(0, minY - patch->height() + 1), min(maxY, outputMask->height() - patch->height() + 1));
        } while (!checkOffset(offX, offY));

        return Point(offX, offY);
    }
}

// Returns true if overlap is wide enough
bool RandomOffsetChooser::checkOffset(const int offX, const int offY) const {

    const Mat patchMask = Mat(*outputMask, Rect(offX, offY, patch->width(), patch->height()));

    int minX, maxX, minY, maxY;
    getBoundaries(patchMask, &minX, &maxX, &minY, &maxY);

    return (maxX - minX >= OVERLAP_WIDTH && maxY - minY >= OVERLAP_WIDTH);
}
