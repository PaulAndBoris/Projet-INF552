//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#ifndef GRAPHCUT_TEXTURES_PATCHER_H
#define GRAPHCUT_TEXTURES_PATCHER_H

#define CAP_TYPE float
#define GRAPH_TYPE Graph<CAP_TYPE, CAP_TYPE, CAP_TYPE>

#include "image.h"
#include "OffsetChooser.h"
#include "maxflow/graph.h"

typedef struct Seam{
    CAP_TYPE cost;
    Vec3b point_1;
    Vec3b neighbor_1;
    Vec3b point_2;
    Vec3b neighbor_2;
} Seam;


class Patcher {

private:
    const Image<Vec3b> patch;
    const Rect canvasRect;
    Image<Vec3b> output;
    Image<uchar> outputMask;
    Seam *oldSeams;

    OffsetChooser *offsetChooser;

    GRAPH_TYPE *buildGraphForOffset(const Point &offset) const;
    int seamIndex(const Point &outputPoint, char direction) const;
    int nodeIndex(const Point &patchPoint, char direction = 0) const;
    Point translatePoint(const Point &pt, char direction) const;
    float edgeWeight(const Point &patchPoint, const Point &outputPoint, char direction) const;
    float edgeWeight(const Vec3b &As, const Vec3b &Bs, const Vec3b &At, const Vec3b &Bt) const ;

public:
    Patcher(const Image<Vec3b> &patch, int width, int height);
    ~Patcher();

    const Image<Vec3b> step();

};


#endif //GRAPHCUT_TEXTURES_PATCHER_H
