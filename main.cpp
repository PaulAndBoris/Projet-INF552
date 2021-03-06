//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "image.h"
#include "Patcher.h"

int main() {

    const Image<Vec3b> patch = imread("../patch3.png", IMREAD_COLOR);
    Patcher patcher(patch, 1000, 800);

    do {
        imshow("Result", patcher.step());
    } while (waitKey(0) != 27);

    return 0;
}
