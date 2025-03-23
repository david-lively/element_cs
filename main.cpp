#include <iostream>

using namespace std;

#include "Analyzer.h"

bool inRange(const Vec2& vec, float mn, float mx) {
    return vec.x >= mn && vec.x <= mx && vec.y >= mn && vec.y <= mx;
}

void runTests(const Vec2& mapDims, const vector<unsigned char>& beforeData, const vector<unsigned char>& afterData) {
    Vec2 p0(0,0);
    Vec2 p1(255,255);
    Vec2 p2(512,512);

    float d01 = Analyzer::CalculatePathLength(beforeData, mapDims, p0,p1);
    float d12 = Analyzer::CalculatePathLength(beforeData, mapDims, p1,p2);
    float d02 = Analyzer::CalculatePathLength(beforeData,mapDims,p2,p0);

    cout << "d01 = " << d01 << " d12 = " << d12 << " d02 = " << d02 << endl;
    float delta = (d02 - (d01 + d12));
    cout << "delta = " << delta << endl;
}

int main(int argc, char** argv)
{
    cout << "Starting up." << endl;

    Analyzer analyzer;
    Vec2 mapDims(512,512);
    cout << "Loading maps...";
    auto beforeData = Analyzer::LoadHeightmap("data/st-helens/pre.data");
    auto afterData = Analyzer::LoadHeightmap("data/st-helens/post.data");
    cout << "Done." << endl;
    runTests(mapDims, beforeData, afterData);

    Vec2 start;
    Vec2 end;
    cout << "Enter four integer values in x0 y0 x1 y1 order. Ctrl+C to exit." << endl;
    while (true) {
        cout << ">";
        cin >> start.x;// >> start.y >> end.x >> end.y;

        if (start.x == -1)
            break;

        cin >> start.y >> end.x >> end.y;

        cout << "Parsed coordinates" << endl << start.x << "," << start.y << endl << end.x << "," << end.y << endl;
        if (!inRange(start,0,511) || !inRange(end,0,511)) {
            cout << "Invalid coordinates. Use values between 0,0 and " << mapDims.x-1 << "," << mapDims.y-1 << endl;
            continue;
        }
        cout << "Calculating each path separately" << endl;
        float preDistance = Analyzer::CalculatePathLength(beforeData, mapDims, start,end);
        float postDistance = Analyzer::CalculatePathLength(afterData, mapDims,start,end);
        cout << "Before eruption: " << preDistance << endl;
        cout << "After eruption: " << postDistance << endl;
        cout << "Delta: " << postDistance - preDistance << endl;
    }

    cout << "Done." << endl;
    return 0;
}