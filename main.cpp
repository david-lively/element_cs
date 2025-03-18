#include <iostream>

using namespace std;

#include "Analyzer.h"

int main(int argc, char** argv)
{
    cout << "Starting up." << endl;

    Analyzer analyzer;
    analyzer.Load(Vec2(512,512),"data/st-helens/pre.data","data/st-helens/post.data");

    cout << "Enter four integer values for x0 y0 x1 y1" << endl << ">";
    Vec2 start;
    Vec2 end;
    cin >> start.x >> start.y >> end.x >> end.y;

    cout << "Parsed coordinates " << start.x << "," << start.y << endl << end.x << "," << end.y << endl;

    float distance = analyzer.CalculatePathLength(analyzer.data(),start,end);
    cout << "Path length: " << distance << endl;

    cout << "Done." << endl;
    return 0;
}