//
// Created by Paul VANHAESEBROUCK on 25/11/2015.
//

#include <iostream>
#include <opencv2/highgui.hpp>
#include "Patcher.h"

#include "maxflow/graph.h"

enum Direction {
    NONE = 0,
    RIGHT,
    LEFT,
    UP,
    BOTTOM,
    LAST
};

Patcher::Patcher(const Image<Vec3b> &patch, int width, int height) :
        patch(patch),
        canvasRect(patch.width(), patch.height(), width, height),
        output(Mat::zeros(height + 2 * patch.height(), width + 2 * patch.width(), CV_8UC3)),
        outputMask(Mat::zeros(height + 2 * patch.height(), width + 2 * patch.width(), CV_8U)),
        rndOffsetChooser(&this->patch, &this->outputMask) {

    oldSeams = new CAP_TYPE[(width + 2 * patch.width()) * (height + 2 * patch.height()) * 2];
    for (int i = 0; i < (width + 2 * patch.width()) * (height + 2 * patch.height()) * 2; oldSeams[i] = -1.f, i++);
}

Patcher::~Patcher() {
    delete[] oldSeams;
}

// Points & direction utilities

int Patcher::nodeIndex(const Point &patchPoint, char direction) const {
    switch (direction) {
        case NONE:
            return patchPoint.y * patch.width() + patchPoint.x + 0;
        case RIGHT: //Doesn't work if the node is on the right edge
            return patchPoint.y * patch.width() + patchPoint.x + 1;
        case LEFT: //Doesn't work if the node is on the left edge
            return patchPoint.y * patch.width() + patchPoint.x - 1;
        case UP: //Doesn't work if the node is on the top edge
            return (patchPoint.y - 1) * patch.width() + patchPoint.x;
        case BOTTOM: //Doesn't work if the node is on the bottom edge
            return (patchPoint.y + 1) * patch.width() + patchPoint.x;
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
        case LEFT:
            return 2 * (outputPoint.y * output.width() + outputPoint.x) + 0;
        case UP:
            return 2 * (outputPoint.y * output.width() + outputPoint.x) + 1;
        default:
            printf("Patcher::seamIndex direction is not valid.\n");
            return -1;
        }
}

Point Patcher::translatePoint(const Point &pt, char direction) const {
    switch (direction) {
        case NONE:
            return pt;
        case RIGHT:
            return Point(pt.x + 1, pt.y);
        case LEFT:
            return Point(pt.x -1, pt.y);
        case UP:
            return Point(pt.x, pt.y - 1);
        case BOTTOM:
            return Point(pt.x, pt.y + 1);
        default:
            printf("Patcher::translatePoint direction is not valid.\n");
            return Point(-1, -1);
    }
}

// Main functions

const Image<Vec3b> Patcher::randomStep() {


    bool foundMask;
    Point offset = rndOffsetChooser.getNewOffset(&foundMask);

    Image<uchar> patchMask(Mat::zeros(patch.height(), patch.width(), CV_8U) + 255);

    if (foundMask) {
        GRAPH_TYPE *graph;
        CAP_TYPE flow;

       // do {
            offset = rndOffsetChooser.getNewOffset(&foundMask);
            graph = buildGraphForOffset(offset);

            cout<<"nombre de noeuds : "<<graph->get_node_num()<<endl;
        cout<<"nombre d'arretes : "<<graph->get_arc_num()<<endl;
            flow = graph->maxflow();
       // } while (flow > 300);

        cout << "Flow = " << flow << endl;

        Point pt;

        for (pt.y = 0; pt.y < patch.height(); pt.y++)
            for (pt.x = 0; pt.x < patch.width(); pt.x++) {

                patchMask(pt) = (uchar) ((graph->what_segment(nodeIndex(pt)) == GRAPH_TYPE::SOURCE && (outputMask(pt+offset))) ? 0 : 255);

                // TODO: Update oldSeam array
//            if (pt.y < patch.height() - 1 && pt.x < patch.width() - 1)
//                for (char direction = RIGHT; direction != LAST; direction++)
//                    oldSeams[seamIndex(offset + pt)] = (patchMask(pt)) ?  : -1;
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

void Patcher::setBoundaries(const Point & offset, GRAPH_TYPE *graph, bool* linkedNodes) const{

    Point patchPoint;


    //Points on the edge of the patch and in the output mask must come from the original image
    patchPoint.x = 0;
    for (patchPoint.y = 0; patchPoint.y < patch.height(); patchPoint.y++) {
        const Point outputPoint = offset + patchPoint;
        const int node = nodeIndex(patchPoint);
        if(outputMask(outputPoint)){
            graph->add_tweights(node, INFINITY , 0);
            linkedNodes[node] = true;
        }

    }
    patchPoint.x = patch.width() -1;
    for (patchPoint.y = 0; patchPoint.y < patch.height(); patchPoint.y++) {
        const Point outputPoint = offset + patchPoint;
        const int node = nodeIndex(patchPoint);
        if (outputMask(outputPoint)) {
            graph->add_tweights(node, INFINITY, 0);
            linkedNodes[node] = true;
        }
    }

    patchPoint.y = 0;
    for (patchPoint.x = 0; patchPoint.x < patch.width(); patchPoint.x++) {
        const Point outputPoint = offset + patchPoint;
        const int node = nodeIndex(patchPoint);
        if (outputMask(outputPoint)) {
            graph->add_tweights(node, INFINITY, 0);
            linkedNodes[node] = true;
        }
    }

    patchPoint.y = patch.height() -1;
    for (patchPoint.x = 0; patchPoint.x < patch.width(); patchPoint.x++) {
        const Point outputPoint = offset + patchPoint;
        const int node = nodeIndex(patchPoint);
        if (outputMask(outputPoint)) {
            graph->add_tweights(node, INFINITY, 0);
            linkedNodes[node] = true;
        }
    }

    //Points on the edge of the output mask and in the patch belong to the patch
    //These points are the neighbors of patch points not in the mask which are in the output mask
    //CAUTION : these neighbors can be right, left, bottom and up !

    for (patchPoint.y = 1; patchPoint.y < patch.height() - 1 ; patchPoint.y++) {
        for (patchPoint.x = 1; patchPoint.x < patch.width() - 1; patchPoint.x++) {

            const Point outputPoint = offset + patchPoint;

            for (char direction = RIGHT; direction != LAST; direction++) {

                const Point neighborPoint = translatePoint(outputPoint, direction);
                const int neighborNode = nodeIndex(patchPoint, direction);

                if (!outputMask(outputPoint) && outputMask(neighborPoint) && !linkedNodes[neighborNode]) {
                    graph->add_tweights(neighborNode, 0, INFINITY);
                    linkedNodes[neighborNode] = true;

                    }
                }
            }
        }
    }


GRAPH_TYPE *Patcher::buildGraphForOffset(const Point &offset) const {

    GRAPH_TYPE *graph = new GRAPH_TYPE(patch.width() * patch.height(), patch.width() * patch.height() * 2);

    graph->add_node(patch.width() * patch.height());
    int lastNode = patch.width() * patch.height();

    int edge_count = 0;

    bool linkedNodes[patch.width() * patch.height()];
    memset(linkedNodes, false, patch.width() * patch.height() * sizeof(bool));

    Point patchPoint;

    setBoundaries(offset, graph, linkedNodes);

    for (patchPoint.y = 1; patchPoint.y < patch.height() - 1 ; patchPoint.y++) {
        for (patchPoint.x = 1; patchPoint.x < patch.width() - 1 ; patchPoint.x++) {

            const Point outputPoint = offset + patchPoint;
            const int node = nodeIndex(patchPoint);

            for (char direction = RIGHT; direction != LAST; direction++) {

                const Point neighborPoint = translatePoint(outputPoint, direction);
                const int neighborNode = nodeIndex(patchPoint, direction);


                if (outputMask(outputPoint) && outputMask(neighborPoint)) {

                    const int seam = seamIndex(outputPoint, direction);
                    const CAP_TYPE cap = edgeWeight(patchPoint, outputPoint, direction);


                    if (oldSeams[seam] >= 0) {
                        printf("Ca c'est pas normal\n");
                        graph->add_node();

                        graph->add_edge(node, lastNode, cap, cap);
                        graph->add_edge(lastNode, neighborNode, cap, cap);
                        graph->add_tweights(lastNode, 0, oldSeams[seam]);

                        lastNode++;
                    } else {
                        graph->add_edge(node, neighborNode, cap, cap);
                        edge_count++;
                    }
                }
            }
        }
    }
    printf("Nombre d'arrêtes : %f" , (double) edge_count);
//    printf("\nNombre de liens : %f " ,(double) link_count);



//    Test
//
//    Image<uchar> testMask(Mat::zeros(patch.height(), patch.width(), CV_8U));
//
//    for (patchPoint.y = 0; patchPoint.y < patch.height(); patchPoint.y++)
//        for (patchPoint.x = 0; patchPoint.x < patch.width(); patchPoint.x++)
//            if (linkedNodes[nodeIndex(patchPoint)])
//                testMask(patchPoint) = 255;
//
//    imshow("test", testMask);
//
//    Test end

    return graph;
}

CAP_TYPE Patcher::edgeWeight(const Point &patchPoint, const Point &outputPoint, char direction) const {

    return norm(patch(patchPoint) - output(outputPoint)) +
           norm(patch(translatePoint(patchPoint , direction)) - output(translatePoint(outputPoint, direction)));
}
