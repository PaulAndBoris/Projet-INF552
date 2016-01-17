//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include "PatchMatcher.h"

#define OVERLAP_WIDTH (1<<4)
#define K 1.0

PatchMatcher::PatchMatcher(const Image<Vec3b> *patch, const Image<Vec3b> *output, const Image<uchar> *outputMask) :
    OffsetChooser(),
    patch(patch),
    output(output),
    outputMask(outputMask),
    nextOffset(0, 0) {

    Scalar mean, stddev;
    meanStdDev(*patch, mean, stddev);

    factor = - 1.0 / (K * pow(norm(stddev), 2) * patch->width() * patch->height());
}

Point PatchMatcher::getNewOffset(Image<Vec3b> *newPatch, bool *foundMask) {

    *foundMask = nextOffset != Point(0, 0);

    if (nextOffset.x >= 0 && nextOffset.y >= 0) {

        const Point windowOffset = nextOffset;

        nextOffset.x += patch->width() - OVERLAP_WIDTH;
        if (nextOffset.x >= output->width()) {
            nextOffset.x = 0;
            nextOffset.y += patch->height() - OVERLAP_WIDTH;
        }
        if (nextOffset.y >= output->height()) {
            nextOffset.x = -1;
            nextOffset.y = -1;
        }

        *newPatch = ((Mat) *patch)(Rect(0, 0, min(patch->width(), output->width() - windowOffset.x), min(patch->height(), output->height() - windowOffset.y)));

        return windowOffset;

    } else {

        *newPatch = *patch;

        Image<float> C;

        matchTemplate(*patch, *output, C, TM_SQDIFF);

        exp(C * factor, C);

        double min, max;
        Point maxLoc;
        minMaxLoc(C, &min, &max, 0, &maxLoc);

        imshow("C", C.greyImage());

        float acc = 0, *data = (float *) C.data;

        for (int i = 0; i < C.height() * C.width(); i++) {
            acc += data[i];
            data[i] = acc;
        }

        int a = 0, b = C.height() * C.width() - 1;
        const float p = data[b] * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

        // Dichotomy search
        while (b - a > 0) {
            const int m = (a + b) / 2;
            if (p > data[m])
                a = m + 1;
            else
                b = m;
        }

        return Point(a % C.width(), a / C.width());
//        return maxLoc;
    }
}
