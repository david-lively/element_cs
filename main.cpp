#include <iostream>

using namespace std;

#include "Analyzer.h"

bool inRange(const Vec2& vec, float mn, float mx) {
    return vec.x >= mn && vec.x <= mx && vec.y >= mn && vec.y <= mx;
}

void runTests(const Vec2& mapDims, const vector<unsigned char>& beforeData, const vector<unsigned char>& afterData) {

    {
        cout << "Split diagonal test" << endl;
        Vec2 p0(0,0);
        Vec2 p1(255,255);
        Vec2 p2(511,511);
        cout << "P0 " << p0.x << "," << p0.y << endl;
        cout << "P1 " << p1.x << "," << p1.y << endl;
        cout << "P2 " << p2.x << "," << p2.y << endl;

        float d01 = Analyzer::CalculatePathLength(beforeData, mapDims, p0,p1);
        float d12 = Analyzer::CalculatePathLength(beforeData, mapDims, p1,p2);
        float d02 = Analyzer::CalculatePathLength(beforeData,mapDims,p2,p0);

        cout << "d01 = " << d01 << " d12 = " << d12 << " d02 = " << d02 << endl;
        float error = d02 - d01 - d12;
        cout << "error = " << error << endl;
    }

    cout << "\n\n";

    {
        Vec2 start(0,0);
        Vec2 end(511,0);
        float cp0 = 0;
        float cp1 = 0;
        for (int i=start.x + 1; i <= end.x; ++i) {
            float d0 = 11.0f * abs(beforeData[i] - beforeData[i-1]);
            cp0 += sqrt(30*30+d0*d0);
            float d1 = 11.0f * abs(afterData[i] - afterData[i-1]);
            cp1 += sqrt(30*30 + d1*d1);
        }

        cout << " Horizontal test\n: Before: " << cp0 << " after " << cp1 << " delta " << (cp1 - cp0) << endl;
        float ap0 = Analyzer::CalculatePathLength(beforeData,mapDims,start, end);
        float ap1 = Analyzer::CalculatePathLength(afterData,mapDims,start,end);
        cout << "Calculated:\n  Before: " << ap0 << " after " << ap1 << " delta " << ap1 - ap0 << endl;

        cout << "Compare Horizontal test from " << start.x << "," << start.y << " to " << end.x << "," << end.y << endl;
        cout << "Before\tAfter\tDelta\n";
        cout << cp0 << "\t" << cp1 << "\t" << (cp0 - cp1) << endl;
        cout << ap0 << "\t" << ap1 << "\t" << (ap0 - ap1) << endl;
        cout << "-----------------------------------------------------" << endl;
        cout << (cp0 - ap0) << "\t" << (cp1 - ap1) << "\t" << (cp1 - cp0) - (ap1 - ap0) << endl;
    }
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