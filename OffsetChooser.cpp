//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#include "OffsetChooser.h"


OffsetChooser::~OffsetChooser() {

}

OffsetChooser::OffsetChooser() {

    // Initialise random number generator
    srand((unsigned int) time(NULL));
}

// Computes boundaries of the non-null area in mask`
void OffsetChooser::getBoundaries(const Image<uchar> &mask, int *minX, int *maxX, int *minY, int *maxY) {

    *minX = mask.width();
    *maxX = -1;
    *minY = mask.height();
    *maxY = -1;

    int x, y;

    // Iterate on lines
    for (y = 0; y < mask.height(); y++) {

        for (x = 0; x < mask.width(); x++)
            if (mask(x, y)) {
                if (x < *minX)
                    *minX = x;
                break;
            }

        for (x++; x < mask.width(); x++)
            if (!mask(x, y)) {
                if (x > *maxX)
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
            if (mask(x, y)) {
                if (y < *minY)
                    *minY = y;
                break;
            }

        for (y++; y < mask.height(); y++)
            if (!mask(x, y)) {
                if (y > *maxY)
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
int OffsetChooser::randRange(const int min, const int max) {
    return rand() % (max - min) + min;
}

