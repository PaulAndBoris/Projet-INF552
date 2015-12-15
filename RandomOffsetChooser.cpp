//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include "RandomOffsetChooser.h"

#define OVERLAP_WIDTH 1<<3

RandomOffsetChooser::RandomOffsetChooser(const Image<Vec3b> *patch, const Image<uchar> *outputMask) :
        patch(patch),
        outputMask(outputMask) {

    // Initialise random number generator
    srand((unsigned int) time(NULL));
}

// Returns a valid offset for the new patch to apply
const Point RandomOffsetChooser::getNewOffset(bool *foundMask) const {

    int minX, maxX, minY, maxY;
    getBoundaries(*outputMask, &minX, &maxX, &minY, &maxY);

    if (maxX < 0 || maxY < 0) {
        *foundMask = false;
        return Point(randRange(0, outputMask->width()  - patch->width()  + 1),
                     randRange(0, outputMask->height() - patch->height() + 1));
    } else {
        *foundMask = true;

        int offX, offY;
     //   do {
            offX = randRange(max(0, minX - patch->width()  + 1), min(maxX, outputMask->width()  - patch->width()  + 1));
            offY = randRange(max(0, minY - patch->height() + 1), min(maxY, outputMask->height() - patch->height() + 1));
       // } while (!checkOffset(offX, offY));

        return Point(offX, offY);
    }
}

// Computes boundaries of the non-null area in mask`
void RandomOffsetChooser::getBoundaries(const Image<uchar> &mask, int *minX, int *maxX, int *minY, int *maxY) const {

    *minX = mask.width();
    *maxX = -1;
    *minY = mask.height();
    *maxY = -1;

    int x, y;

    // Iterate on lines
    for (y = 0; y < mask.height(); y++) {

        for (x = 0; x < mask.width(); x++)
            if (mask(x, y) && x < *minX) {
                *minX = x;
                break;
            }

        for (x++; x < mask.width(); x++)
            if (!mask(x, y) && x > *maxX) {
                *maxX = x-1;
                break;
            }
    }

    if (*minX == mask.width()) // No pixel found, we can return already
        return;
    else if (*maxX < *minX) // Mask touches the border of the image
        *maxX = mask.width() - 1;


    // Iterate on columns
    for (x = 0; x < mask.width(); x++) {

        for (y = 0; y < mask.height(); y++)
            if (mask(x, y) && y < *minY) {
                *minY = y;
                break;
            }

        for (y++; y < mask.height(); y++)
            if (!mask(x, y) && y > *maxY) {
                *maxY = y-1;
                break;
            }
    }

    if (*minY == mask.height()) // No pixel found, we can return already
        return;
    else if (*maxY < *minY) // Mask touches the border of the image
        *maxY = mask.height() - 1;
}

// Returns random integer value in [min, max[
inline int RandomOffsetChooser::randRange(const int min, const int max) const {
    return rand() % (max - min) + min;
}

// Returns true if overlap is wide enough
bool RandomOffsetChooser::checkOffset(const int offX, const int offY) const {

    const Mat patchMask = Mat(*outputMask, Rect(offX, offY, patch->width(), patch->height()));

    int minX, maxX, minY, maxY;
    getBoundaries(patchMask, &minX, &maxX, &minY, &maxY);

    return (maxX - minX >= OVERLAP_WIDTH && maxY - minY >= OVERLAP_WIDTH);
}
