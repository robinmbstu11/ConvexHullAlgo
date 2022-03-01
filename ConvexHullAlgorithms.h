#ifndef CODE_CONVEXHULLALGORITHMS_H
#define CODE_CONVEXHULLALGORITHMS_H

#include "CGALComponents.h"
#include <QtWidgets>
#include <QApplication>
#include <QLabel>
#include <QString>
#include <QTranslator>
#include <CGAL/enum.h>
#include <algorithm>
#include <cstdio>

void drawConvexHullUsingQT(const std::vector<Point> &P,
                        const std::vector<Point> &convexHullPoints,
                        const bool labels) {

    const double pointSize = 4; // adjust this value to change the size of the points
    /***************************************************/
    QPicture pi;
    QPainter canvas(&pi);
    canvas.setRenderHint(QPainter::Antialiasing);
    //canvas.setFont(QFont("Times", 12));
    // DO NOT TOUCH THESE Qt STATEMENTS!!!
    /***************************************************/

    std::vector<QPointF> verticesForQTConvexHull;
    verticesForQTConvexHull.reserve(convexHullPoints.size());
    for( Point p : convexHullPoints )
        verticesForQTConvexHull.emplace_back( QPointF(p.x(),p.y() ) );

    canvas.drawPolygon(&verticesForQTConvexHull[0], (int)verticesForQTConvexHull.size());

    unsigned id = 0;
    for ( Point p : P ) {
        canvas.setBrush(Qt::black);

        canvas.drawEllipse(QPointF(p.x(), p.y()), pointSize, pointSize);
        if(labels) {
            QPointF textPos(p.x() + 4.0, p.y() + 4.0);
            canvas.drawText(textPos, QString(std::to_string(id).c_str()));
        }
        id++;
    }

    for ( Point p : convexHullPoints ) {
        canvas.setBrush(Qt::blue);
        canvas.drawEllipse(QPointF(p.x(), p.y()), pointSize, pointSize);
    }

    /*********************************************/
    canvas.end();
    auto l = new QLabel;
    l->setStyleSheet("QLabel { background-color : white; color : black; }");
    l->setPicture(pi);
    l->setWindowTitle("Polygon Triangulation");
    l->show();
    // l->showMaximized();
    QApplication::exec();
    // DO NOT TOUCH THESE Qt STATEMENTS!!!
    /***************************************************/
}



bool distance(Point a, Point b, Point c) {

    double distAToB = CGAL::squared_distance(a,b);
    double distAToC = CGAL::squared_distance(a,c);
    return distAToB < distAToC;
}

void jarvisMarch(const std::vector<Point> &P, std::vector<unsigned> &idsOfConvexHullPoints) {
    // complete
    // Warning: the point ids in idsOfConvexHullPoints must be in either clockwise or anticlockwise order

    auto M = createMapOfPoints(P);
    Point start = P[0];

    for( auto x: P )
    {
        if( x.x() < start.x() )
        {
            start = x;
        }
    }

    Point current = start;

    std::set<Point> jarvisMarchHull;

    jarvisMarchHull.insert(start);

    std::vector<Point> collinearPoints;

    while(true)
    {
        Point nextTarget = P[0];
        for( unsigned i=1; i<P.size(); i++ )
        {
            if( P[i] == current ) continue;

            if( CGAL::orientation(current,nextTarget,P[i]) == CGAL::LEFT_TURN )
            {
                nextTarget = P[i];
                collinearPoints.clear();
            }
            else if( CGAL::orientation(current,nextTarget,P[i]) == 	CGAL::COLLINEAR )
            {
                if( distance(current,nextTarget,P[i]) )
                {
                    collinearPoints.emplace_back(nextTarget);
                    nextTarget = P[i];
                }
                else {
                    collinearPoints.emplace_back(P[i]);
                }
            }
        }

        for( auto x: collinearPoints ) jarvisMarchHull.insert(x);

        if( nextTarget == start ) break;

        jarvisMarchHull.insert(nextTarget);
        current = nextTarget;
    }
    std::vector<Point> draw(jarvisMarchHull.begin(),jarvisMarchHull.end());

    for( auto x: jarvisMarchHull )
    {
        idsOfConvexHullPoints.emplace_back(M[x]);
        //std::cout<<M[x]<<std::endl;
    }

    //drawConvexHullUsingQT(P,draw,true);
}

bool compareXCord(Point a, Point b)
{
    return a.x() < b.x();
}

Point nextToTop(std::stack<Point> &S)
{
    Point p = S.top();
    S.pop();
    Point res = S.top();
    S.push(p);

    return res;
}

void grahamsScan(std::vector<Point> &P, std::vector<unsigned> &idsOfConvexHullPoints) {
    // complete
    // Warning: the point ids in idsOfConvexHullPoints must be in either clockwise or anticlockwise order
    auto M = createMapOfPoints(P);
    sort(P.begin(),P.end(), compareXCord);

    std::stack<Point> lUpper;

    lUpper.push(P[0]);
    lUpper.push(P[1]);

    for( unsigned i = 2;  i<P.size(); i++ )
    {
        while( lUpper.size() > 1 && CGAL::orientation(nextToTop(lUpper),lUpper.top(),P[i]) != CGAL::RIGHT_TURN )
        {
            lUpper.pop();
        }
        lUpper.push(P[i]);
    }

    std::vector<Point> grahamsScanHull;
    while( !lUpper.empty() )
    {
        grahamsScanHull.emplace_back(lUpper.top());
        idsOfConvexHullPoints.emplace_back(M[lUpper.top()]);
        lUpper.pop();
    }

    std::stack<Point> lLower;

    lLower.push(P[P.size()-1]);
    lLower.push(P[P.size()-2]);

    for(unsigned i = P.size()-2; i>=1; i--)
    {
        while( lLower.size() > 1 && CGAL::orientation(nextToTop(lLower),lLower.top(),P[i]) != CGAL::RIGHT_TURN )
        {
            lLower.pop();
        }
        lLower.push(P[i]);
    }

    while(!lLower.empty() )
    {
        grahamsScanHull.emplace_back(lLower.top());
        idsOfConvexHullPoints.emplace_back(M[lLower.top()]);
        lLower.pop();
    }
    //drawgrahamsScanHullUsingQT(P,grahamsScanHull,true);
}


void plot() {
    // should generate two files a.txt and b.txt
    // a.txt is meant for points generated inside a square (use generatePointsInsideASquare(..) to generate those)
    // b.txt is meant for convex point sets (use generateConvexPointSetsInsideASquare(..) to generate those)
    // Warning: first column must contain the n-values, the second column must contain
    // Graham's Scan runtimes and the third column must contains Jarvis March runtimes

    // use values n = 1K, 2K, ...., 20K, and for every value of n,
    // use 5 pointsets and take the average of 5 runtimes

    //std::freopen("a.txt","w",stdout);

    for( unsigned i = 1; i<=20; i++ ) {
        double grahamsScanRuntime = 0.0;
        double jarvisMarchRuntime = 0.0;
        unsigned numberOfPoints;
        for( unsigned j =0 ; j<5; j++ ) {
            numberOfPoints = i * 1000;
            unsigned sizeOfSquare = 500;
            std::vector<Point> P;
            generatePointsInsideASquare(numberOfPoints, sizeOfSquare, P);
            //generateConvexPointSetInsideASquare(numberOfPoints,sizeOfSquare,P);

            CGAL::Timer clock;

            std::vector<unsigned> idsOfConvexHullPoints;
            clock.start();
            grahamsScan(P, idsOfConvexHullPoints);
            clock.stop();
            grahamsScanRuntime += clock.time();

            clock.reset();

            clock.start();
            jarvisMarch(P,idsOfConvexHullPoints);
            clock.stop();

            jarvisMarchRuntime += clock.time();

        }
        std::cout<<numberOfPoints<<" "<<grahamsScanRuntime/5.0<<" "<<jarvisMarchRuntime/5.0<<std::endl;
    }

    //std::freopen("b.txt","w",stdout);

    for( unsigned i = 1; i<=20; i++ ) {
        double grahamsScanRuntime = 0.0;
        double jarvisMarchRuntime = 0.0;
        unsigned numberOfPoints;
        for( unsigned j =0 ; j<5; j++ ) {
            numberOfPoints = i * 1000;
            unsigned sizeOfSquare = 300;
            std::vector<Point> P;
            generateConvexPointSetInsideASquare(numberOfPoints,sizeOfSquare,P);

            CGAL::Timer clock;

            std::vector<unsigned> idsOfConvexHullPoints;
            clock.start();
            grahamsScan(P, idsOfConvexHullPoints);
            clock.stop();
            grahamsScanRuntime += clock.time();

            clock.reset();

            clock.start();
            jarvisMarch(P,idsOfConvexHullPoints);
            clock.stop();

            jarvisMarchRuntime += clock.time();

        }
        std::cout<<numberOfPoints<<" "<<grahamsScanRuntime/5.0<<" "<<jarvisMarchRuntime/5.0<<std::endl;
    }
}

#endif //CODE_CONVEXHULLALGORITHMS_H
