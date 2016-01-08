//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include "Patcher.h"

#include "maxflow/graph.h"

#include "RandomOffsetChooser.h"
#include "PatchMatcher.h"
#include "SubPatchMatcher.h"


Patcher::Patcher(const Image<Vec3b> &patch, int width, int height) :
        inputPatch(patch),

//        canvasRect(patch.width(), patch.height(), width + 2 * patch.width(), height + 2 * patch.height()),
        canvasRect(0, 0, width, height),
//        output(Mat::zeros(height + 2 * patch.height(), width + 2 * patch.width(), CV_8UC3)),
        output(Mat::zeros(height, width, CV_8UC3)),

        outputMask(Mat::zeros(height + 2 * patch.height(), width + 2 * patch.width(), CV_8U)) {

//    offsetChooser = new RandomOffsetChooser(&inputPatch, &outputMask);
//    offsetChooser = new PatchMatcher(&inputPatch, &output, &outputMask);
    offsetChooser = new SubPatchMatcher(&inputPatch, &output, &outputMask, this);

    oldSeams = new Seam[2 * output.width() * output.height()];
    for (int i = 0; i < 2 * output.width() * output.height(); oldSeams[i].cost = 0.f, i++);
}

Patcher::~Patcher() {
    delete offsetChooser;
    delete[] oldSeams;
}

// Points & direction utilities

int Patcher::nodeIndex(const Point &patchPoint, char direction, const Image<Vec3b> &patch) const {
    switch (direction) {
        case NONE:
            return patchPoint.y * patch.width() + patchPoint.x + 0;
        case RIGHT:
            return patchPoint.y * patch.width() + patchPoint.x + 1;
        case BOTTOM:
            return patchPoint.y * patch.width() + patchPoint.x + patch.width();
        default:
            printf("Patcher::nodeIndex direction is not valid.\n");
            return -1;
    }
}

int Patcher::seamIndex(const Point &outputPoint, char direction) const {
    switch (direction) {
        case RIGHT:
            return 2 * (outputPoint.y * output.width() + outputPoint.x) + 0;
        case BOTTOM:
            return 2 * (outputPoint.y * output.width() + outputPoint.x) + 1;
        default:
            printf("Patcher::seamIndex direction is not valid.\n");
            return -1;
        }
}

Point Patcher::translatePoint(const Point &pt, char direction) {
    switch (direction) {
        case NONE:
            return pt;
        case RIGHT:
            return Point(pt.x + 1, pt.y);
        case BOTTOM:
            return Point(pt.x, pt.y + 1);
        default:
            printf("Patcher::translatePoint direction is not valid.\n");
            return Point(-1, -1);
    }
}

// Main functions

const Image<Vec3b> Patcher::step() {

    bool foundMask;
    Image<Vec3b> patch;

    const Point offset = offsetChooser->getNewOffset(patch, &foundMask);

    Image<uchar> patchMask(Mat::zeros(patch.height(), patch.width(), CV_8U) + 255);

    if (foundMask) {
        GRAPH_TYPE *graph;
        CAP_TYPE flow;

        graph = buildGraphForOffset(offset, patch);
        graph->maxflow();

        Point pt;

        // Create patch mask
        for (pt.y = 0; pt.y < patch.height(); pt.y++)
            for (pt.x = 0; pt.x < patch.width(); pt.x++)
                patchMask(pt) = (uchar) ((graph->what_segment(nodeIndex(pt, 0, patch)) == GRAPH_TYPE::SINK) ? 255 : 0);

        // Updating old seams
        for (pt.y = 0; pt.y < patch.height(); pt.y++)
            for (pt.x = 0; pt.x < patch.width(); pt.x++) {

                const Point outputPoint = offset + pt;

                for (char direction = RIGHT; direction != LAST; direction++) {

                    if ((direction == RIGHT && pt.x == patch.width() - 1) ||
                        (direction == BOTTOM && pt.y == patch.height() - 1))
                        continue;

                    const Point neighborPt = translatePoint(pt, direction);
                    const Point neighborOutputPoint = offset + neighborPt;

                    if (patchMask(pt) != patchMask(neighborPt)) {

                        Seam &seam = oldSeams[seamIndex(outputPoint, direction)];

                        seam.cost = edgeWeight(pt, outputPoint, direction, patch);

                        seam.point_1 = output(outputPoint);
                        seam.neighbor_1 = output(neighborOutputPoint);

                        seam.point_2 = patch(pt);
                        seam.neighbor_2 = patch(neighborPt);
                    }
                    else
                        oldSeams[seamIndex(offset, direction)].cost = 0.f;
                }
            }

        delete graph;
    }

    imshow("patchMask", patchMask);

    const Rect patchRect = Rect(offset, Size(patch.width(), patch.height()));
    patch.copyTo(((Mat) output)(patchRect), patchMask);
    patchMask.copyTo(((Mat) outputMask)(patchRect), patchMask);

    imshow("outputMask", ((Mat) outputMask)(canvasRect));

    return ((Mat) output)(canvasRect);
}

GRAPH_TYPE *Patcher::buildGraphForOffset(const Point &offset, const Image<Vec3b> &patch) const {

    GRAPH_TYPE *graph = new GRAPH_TYPE(patch.width() * patch.height(), patch.width() * patch.height() * 2);

    graph->add_node(patch.width() * patch.height());
    int lastNode = patch.width() * patch.height();

    Point patchPoint;

    for (patchPoint.y = 0; patchPoint.y < patch.height(); patchPoint.y++) {
        for (patchPoint.x = 0; patchPoint.x < patch.width(); patchPoint.x++) {

            const Point outputPoint = offset + patchPoint;
            const int node = nodeIndex(patchPoint, 0, patch);

            if (outputMask(outputPoint)) {
                if (patchPoint.x == 0 ||
                    patchPoint.y == 0 ||
                    patchPoint.x == patch.width() - 1 ||
                    patchPoint.y == patch.height() - 1) {

                    graph->add_tweights(node, INFINITY, 0);
                }
            } else
                graph->add_tweights(node, 0, INFINITY);

            for (char direction = RIGHT; direction != LAST; direction++) {

                if ((direction == RIGHT && patchPoint.x == patch.width() - 1) || (direction == BOTTOM && patchPoint.y == patch.height() - 1))
                    continue;

                // Case 1. pixel is in mask and its neighbor too                   == OVERLAP
                // Case 2. pixel is in mask but not its neighbor                   == END OF OVERLAP -- PATCH SIDE
                // Case 3. pixel is not in mask but its neighbor is                == END OF OVERLAP -- PATCH SIDE
                // Case 4. pixel is on the border of the patch and inside the mask == END OF OVERLAP -- OUTPUT SIDE
                // Case 5. pixel and its neighbor are not in mask                  == NOT IMPORTANT

                const Point neighborPoint = translatePoint(outputPoint, direction);
                const int neighborNode = nodeIndex(patchPoint, direction, patch);

                if (outputMask(outputPoint) && outputMask(neighborPoint)) {

                    const int seam = seamIndex(outputPoint, direction);

                    if (oldSeams[seam].cost > 0) {
                        //Add an seam node
                        graph->add_node();
                        //Connect it to the new patch (e.g. SINK) with the old cost
                        graph->add_tweights(lastNode, 0, oldSeams[seam].cost);
                        //Connect it to the old patch, first side
                        CAP_TYPE cap = edgeWeight(oldSeams[seam].point_1, patch(patchPoint), oldSeams[seam].neighbor_1 , patch(translatePoint(patchPoint,direction))) + DBL_EPSILON;
                        graph->add_edge(node, lastNode, cap, cap);
                        //Connect it to the old patch, second side
                        cap = edgeWeight(oldSeams[seam].point_2, patch(patchPoint), oldSeams[seam].neighbor_2 , patch(translatePoint(patchPoint,direction))) + DBL_EPSILON;
                        graph->add_edge(lastNode, neighborNode, cap, cap);

                        lastNode++;
                    } else {
                        const CAP_TYPE cap =
                                edgeWeight(patchPoint, outputPoint, direction, patch) + DBL_EPSILON;
                        graph->add_edge(node, neighborNode, cap, cap);
                    }
                }

                if (outputMask(outputPoint) && !outputMask(neighborPoint))
                    graph->add_tweights(node, 0, INFINITY);

                if (outputMask(neighborPoint) && !outputMask(outputPoint))
                    graph->add_tweights(neighborNode, 0, INFINITY);
            }
        }
    }

    return graph;
}

inline CAP_TYPE Patcher::edgeWeight(const Point &patchPoint, const Point &outputPoint, char direction,
                                    const Image<Vec3b> &patch) const {

    return norm(patch(patchPoint) - output(outputPoint)) +
           norm(patch(translatePoint(patchPoint, direction)) - output(translatePoint(outputPoint, direction)));
}

inline CAP_TYPE Patcher::edgeWeight(const Vec3b &As, const Vec3b &Bs, const Vec3b &At, const Vec3b &Bt) const {
    return norm(As - Bs) +
           norm(At - Bt);
}

CAP_TYPE Patcher::seamCost(const Point &pt, char direction) const {
    return oldSeams[seamIndex(pt, direction)].cost;
}
