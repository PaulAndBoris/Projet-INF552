//
// Created by Paul VANHAESEBROUCK on 07/01/2016.
//

#include <opencv2/highgui.hpp>
#include "SubPatchMatcher.h"

#define OVERLAP_WIDTH (1<<4)
#define K 1e-5
#define WINDOW_RATIO 0.3
#define NB_OFFSET_ITER (1<<5)

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

Point SubPatchMatcher::getNewOffset(Image<Vec3b> *newPatch, bool *foundMask) {

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
        windowOffset = findGoodOffset();

    if (!(windowOffset.x == 0 && windowOffset.y == 0)) {

        const Rect windowRect = Rect(windowOffset, Size(min(windowWidth , output->width()  - windowOffset.x),
                                                        min(windowHeight, output->height() - windowOffset.y)));

        Image<float> C;

        Mat windowMask, window;
        assert(patch->type() == CV_8UC3);
        assert(output->type() == CV_8UC3);

        window = ((Mat) *output)(windowRect);
        windowMask = ((Mat) *outputMask)(windowRect);

        Scalar mean, stddev;
        meanStdDev(window, mean, stddev, windowMask);

        const double maskSize = norm(sum(windowMask / 255));
        const double factor = - 1.0 / (K * pow(norm(stddev), 2) * maskSize);

        cvtColor(windowMask, windowMask, CV_GRAY2BGR, 3);

        assert(!windowMask.empty());
        assert(window.size == windowMask.size);
        assert(window.size < patch->size);
        assert(patch->type() == window.type());
        assert(patch->type() == windowMask.type());

        matchTemplate(*patch, window, C, TM_SQDIFF, windowMask);

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

//        patchRect = Rect(Point(a % C.width(), a / C.width()), windowRect.size());
        patchRect = Rect(maxLoc, windowRect.size());
    }

    *newPatch = ((Mat) *patch)(patchRect);

    return windowOffset;
}

Point SubPatchMatcher::findGoodOffset() const {

    CAP_TYPE cost, bestCost = -1;
    Point offset, bestOffset, pt;

    for (int n = 0; n < NB_OFFSET_ITER; n++) {

        offset = Point(randRange(0, output->width()  - windowWidth  + 1),
                       randRange(0, output->height() - windowHeight + 1));
        cost = 0;

        //Initial sum
        for (pt.y = 0; pt.y < windowHeight; pt.y++)
            for (pt.x = 0; pt.x < windowWidth; pt.x++) {

                const Point outputPoint = offset + pt;

                for (char direction = RIGHT; direction != LAST; direction++) {

                    if ((direction == RIGHT && pt.x == windowWidth - 1) ||
                        (direction == BOTTOM && pt.y == windowHeight - 1))
                        continue;

                    cost += patcher->seamCost(outputPoint, direction);
                }
            }

        if (cost >= bestCost) {
            cost = bestCost;
            bestOffset = offset;
        }
    }

    return bestOffset;
}
