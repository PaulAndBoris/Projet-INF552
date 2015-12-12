//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#ifndef GRAPHCUT_TEXTURES_PATCHER_H
#define GRAPHCUT_TEXTURES_PATCHER_H

#define CAP_TYPE float
#define GRAPH_TYPE Graph<CAP_TYPE, CAP_TYPE, CAP_TYPE>

#include "image.h"
#include "RandomOffsetChooser.h"
#include "maxflow/graph.h"

class Patcher {

private:
    const Image<Vec3b> patch;
    const Rect canvasRect;
    Image<Vec3b> output;
    Image<uchar> outputMask;
    float *oldSeams;

    RandomOffsetChooser rndOffsetChooser;

    GRAPH_TYPE *buildGraphForOffset(const Point &offset) const;
    int seamIndex(const Point &outputPoint, char direction) const;
    int nodeIndex(const Point &patchPoint, char direction = 0) const;
    Point translatePoint(const Point &pt, char direction) const;
    float edgeWeight(const Point &patchPoint, const Point &outputPoint, char direction) const;
    void setBoundaries(const Point &offset, GRAPH_TYPE *graph, bool* linkedNodes) const;

public:
    Patcher(const Image<Vec3b> &patch, int width, int height);
    ~Patcher();

    const Image<Vec3b> randomStep();

};


#endif //GRAPHCUT_TEXTURES_PATCHER_H
