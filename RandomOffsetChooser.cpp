//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include "RandomOffsetChooser.h"

#define OVLP_WIDTH 4

RandomOffsetChooser::RandomOffsetChooser(const Mat *patch, const Mat *outputMask) :
        patch((Image<Vec3b> *)patch),
        outputMask((Image<uchar> *)outputMask) {

    // Initialise random number generator
    srand((unsigned int) time(NULL));
}

// Returns a valid offset for the new patch to apply
Point const RandomOffsetChooser::getNewOffset() const {

    int minX, maxX, minY, maxY;
    getBoundaries(*outputMask, &minX, &maxX, &minY, &maxY);

    if (minX < 0 || minY < 0)
        return Point(randRange(0, outputMask->width() - patch->width() + 1), randRange(0, outputMask->height() - patch->height() + 1));

    else {

        int offX, offY;
        do {
            offX = randRange(minX - patch->width() + 1, maxX);
            offY = randRange(minY - patch->height() + 1, maxY);
        } while (!checkOffset(offX, offY));

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
                *minX = (int) x;
                break;
            }

        for (x++; x < mask.width(); x++)
            if (!mask(x, y) && x > *maxX) {
                *maxX = (int) x-1;
                break;
            }
    }

    if (*minX == mask.width()-1) // Mask touches the border of the image
        *maxX = *minX;
    else if (minX < 0) // No pixel found, we can return already
        return;

    // Iterate on columns
    for (x = 0; x < mask.width(); x++) {

        for (y = 0; y < mask.height(); y++)
            if (mask(x, y) && y < *minY) {
                *minY = (int) y;
                break;
            }

        for (y++; y < mask.height(); y++)
            if (!mask(x, y) && y > *maxY) {
                *maxY = (int) y-1;
                break;
            }
    }

    if (*minY == mask.height()-1)
        *maxY = *minY;
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

    return (maxX - minX >= OVLP_WIDTH && maxY - minY >= OVLP_WIDTH);
}
