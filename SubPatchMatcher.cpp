//
// Created by Paul VANHAESEBROUCK on 07/01/2016.
//

#include <opencv2/highgui.hpp>
#include <iostream>
#include "SubPatchMatcher.h"

#define OVERLAP_WIDTH (1<<3)
#define K 1e-5
#define WINDOW_RATIO 0.8

SubPatchMatcher::SubPatchMatcher(const Image<Vec3b> *patch, const Image<Vec3b> *output, const Image<uchar> *outputMask, const Patcher *patcher) :
    OffsetChooser(),
    patch(patch),
    output(output),
    outputMask(outputMask),
    patcher(patcher),
    nextOffset(0, 0),
    windowWidth((const int) round(patch->width() * WINDOW_RATIO)),
    windowHeight((const int) round(patch->height() * WINDOW_RATIO)) {

}

Point SubPatchMatcher::getNewOffset(Image<Vec3b> &newPatch, bool *foundMask) {

    *foundMask = nextOffset != Point(0, 0);

    Point windowOffset;
    Rect patchRect(0, 0, windowWidth, windowHeight);

    if (nextOffset.x >= 0 && nextOffset.y >= 0) {

        windowOffset = nextOffset;

        nextOffset.x += windowWidth - OVERLAP_WIDTH;
        if (nextOffset.x >= output->width()) {
            nextOffset.x = 0;
            nextOffset.y += windowHeight - OVERLAP_WIDTH;
        }
        if (nextOffset.y >= output->height()) {
            nextOffset.x = -1;
            nextOffset.y = -1;
        }

    } else
        windowOffset = computeWindowOffset();

    if (!(windowOffset.x == 0 && windowOffset.y == 0)) {

        const Rect windowRect = Rect(windowOffset, Size(min(windowWidth , output->width()  - windowOffset.x),
                                                        min(windowHeight, output->height() - windowOffset.y)));

        Image<float> C;

//        Image<Vec3f> patchF, windowF, windowMaskF;
        Mat windowMask, window;
        assert(patch->type() == CV_8UC3);
        assert(output->type() == CV_8UC3);

//        patch->convertTo(patchF, CV_32FC3);
//        ((Mat) *output)(windowRect).convertTo(windowF, CV_32FC3);
        window = ((Mat) *output)(windowRect);
        windowMask = ((Mat) *outputMask)(windowRect);

        Scalar mean, stddev;
        meanStdDev(window, mean, stddev, windowMask);

        const double maskSize = norm(sum(windowMask / 255));
        const double factor = - 1.0 / (K * pow(norm(stddev), 2) * maskSize);

        cvtColor(windowMask, windowMask, CV_GRAY2BGR, 3);
        imshow("windowMask", windowMask);

        assert(!windowMask.empty());
        assert(window.size == windowMask.size);
        assert(window.size < patch->size);
        assert(patch->type() == window.type());
        assert(patch->type() == windowMask.type());
//        imshow("windowMaskF", windowMaskF);

        matchTemplate(*patch, window, C, TM_SQDIFF, windowMask);

        Image<float> C2;
        exp(C * factor, C2);

        double min, max;
        Point maxLoc;
        minMaxLoc(C2, &min, &max, 0, &maxLoc);

        imshow("C", C2.greyImage());

        float acc = 0, *data = (float *) C2.data;

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

//         Seems to work
//        assert(p <= data[a] && (a == 0 || p > data[a-1]));
//        assert(data[a] == C(a % C.width(), a / C.width()));

        patchRect = Rect(Point(a % C.width(), a / C.width()), windowRect.size());
//        patchRect = Rect(maxLoc, windowRect.size());
    }

    newPatch = ((Mat) *patch)(patchRect);


    return windowOffset;
}

Point SubPatchMatcher::computeWindowOffset() const {

    CAP_TYPE slidingCost = 0, bestCost = 0;
    Point offset(0, 1), bestOffset(0, 0), pt;
    int dx = 0, dy = 1;

    //Initial sum
    for (pt.y = 0; pt.y < windowHeight; pt.y++)
        for (pt.x = 0; pt.x < windowWidth; pt.x++) {

            for (char direction = RIGHT; direction != LAST; direction++) {

                if ((direction == RIGHT && pt.x == windowWidth - 1) ||
                    (direction == BOTTOM && pt.y == windowHeight - 1))
                    continue;

                slidingCost += patcher->seamCost(pt, direction);
            }
        }

    bestCost = slidingCost;

    while (offset.x <= output->width() - windowWidth) {

        if (dy != 0) {
            pt = Point(0, (dy == 1) ? -1 : 0);
            for (pt.x = 0; pt.x < windowWidth; pt.x++)
                for (char direction = RIGHT; direction != LAST; direction++) {

                    if (direction == RIGHT && pt.x == windowWidth - 1)
                        continue;

                    slidingCost -= dy * patcher->seamCost(pt + offset, direction);
                    slidingCost += dy * patcher->seamCost(pt + offset
                                                             + Point(0, windowHeight + ((direction == RIGHT) ? 0 : -1)),
                                                          direction);
                }
        } else if (dx != 0) {
            pt = Point(-1, 0);
            for (pt.y = 0; pt.y < windowHeight; pt.y++)
                for (char direction = RIGHT; direction != LAST; direction++) {

                    if (direction == BOTTOM && pt.y == windowHeight - 1)
                        continue;

                    slidingCost -= patcher->seamCost(pt + offset, direction);
                    slidingCost += patcher->seamCost(pt + offset
                                                        + Point(windowWidth + ((direction == BOTTOM) ? 0 : -1), 0),
                                                     direction);
                }
        }

        if (slidingCost > bestCost) {
            bestCost = slidingCost;
            bestOffset = offset;
        }

        if (dy != 0) {
            if ((dy == -1 && offset.y > 0) || (dy == 1 && offset.y < output->height() - windowHeight))
                offset.y += dy;
            else {
                dy = 0;
                dx = 1;
                offset.x += dx;
            }
        } else if (dx != 0) {
            dx = 0;
            dy = (offset.y == 0) ? 1 : -1;
            offset.y += dy;
        }
    }

    return bestOffset;
}
