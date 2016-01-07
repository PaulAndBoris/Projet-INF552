//
// Created by Paul VANHAESEBROUCK on 06/01/2016.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include "PatchMatcher.h"

#define OVERLAP_WIDTH (1<<3)
#define K 1.0

PatchMatcher::PatchMatcher(const Image<Vec3b> *patch, const Image<Vec3b> *output, const Image<uchar> *outputMask) :
    OffsetChooser(patch, outputMask), output(output) {

    Scalar mean, stddev;
    meanStdDev(*patch, mean, stddev);

    factor = - 1.0 / (K * pow(norm(stddev), 2) * patch->width() * patch->height());
}

Point PatchMatcher::getNewOffset(bool *foundMask) const {

    int minX, maxX, minY, maxY;
    getBoundaries(*outputMask, &minX, &maxX, &minY, &maxY);

    if (maxX < 0 || maxY < 0) {
        *foundMask = false;
        return Point(randRange(0, outputMask->width()  - patch->width()  + 1),
                     randRange(0, outputMask->height() - patch->height() + 1));
    } else {
        *foundMask = true;

        Image<float> C;

        // Compute allowed translations window
        minX = max(0, minX - patch->width()  + OVERLAP_WIDTH);
        minY = max(0, minY - patch->height() + OVERLAP_WIDTH);
        maxX = min(output->width()  - 1, maxX + patch->width()  - OVERLAP_WIDTH);
        maxY = min(output->height() - 1, maxY + patch->height() - OVERLAP_WIDTH);
        const Rect outputRect(minX, minY, maxX - minX + 1, maxY - minY + 1);

        Mat patchF, outputF;
        assert(patch->type() == CV_8UC3);
        assert(output->type() == CV_8UC3);

        patch->convertTo(patchF, CV_32FC3);
        ((Mat) *output)(outputRect).convertTo(outputF, CV_32FC3);

        matchTemplate(outputF, patchF, C, TM_SQDIFF);

        exp(C * factor, C);

//        imshow("C", C.greyImage());

        float acc, *data = (float *) C.data;

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

        // Seems to work
//        assert(p <= data[a] && (a == 0 || p > data[a-1]));
//        assert(data[a] == C(a % C.width(), a / C.width()));

        return Point(minX + a % C.width(), minY + a / C.width());
    }
}
