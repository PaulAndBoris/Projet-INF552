//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#include "image.h"
#include "Patcher.h"

int main() {

    const Image<Vec3b> patch = imread("../patch.png", IMREAD_COLOR);
    Patcher patcher(patch, 1000, 700);

    do {
        imshow("Result", patcher.randomStep());
    } while (waitKey(0) != 27);

    return 0;
}
