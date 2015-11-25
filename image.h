#pragma once

#include <opencv2/imgproc/imgproc.hpp>

using namespace cv;
using namespace std;

template<typename T>
class Image : public Mat {
public:
    // Constructors
    Image() { }

    Image(const Mat &A) : Mat(A) { }

    Image(int w, int h, int type) : Mat(h, w, type) { }

    // Accessors
    inline T operator()(int x, int y) const { return at<T>(y, x); }

    inline T &operator()(int x, int y) { return at<T>(y, x); }

    inline T operator()(const Point &p) const { return at<T>(p.y, p.x); }

    inline T &operator()(const Point &p) { return at<T>(p.y, p.x); }

    //
    inline int width() const { return cols; }

    inline int height() const { return rows; }

    // To display a floating type image
    Image<uchar> greyImage() const {
        double minVal, maxVal;
        minMaxLoc(*this, &minVal, &maxVal);
        Image<uchar> g;
        double scale = 255.0 / (maxVal - minVal);
        convertTo(g, CV_8U, scale, -minVal*scale);
        return g;
    }
};
