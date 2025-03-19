#include <iostream>

using namespace std;

#include "Analyzer.h"

bool inRange(const Vec2& vec, float mn, float mx) {
    return vec.x >= mn && vec.x <= mx && vec.y >= mn && vec.y <= mx;
}

void generateTestData(Analyzer& analyzer) {
    auto& before = analyzer.beforeData();
    before.resize(512*512);
    auto& after = analyzer.afterData();
    after.resize(512*512);

    for (int i=0; i < before.size(); ++i) {
        before[i] = i % 512;
        after[i] = 0;
    }

    float dx = 512 * 30;
    float dh = 11 * 512;
    float expectedPre = sqrt(dx * dx + dh * dh);
    cout << "Expected pre " << expectedPre << endl;

}

int main(int argc, char** argv)
{
    cout << "Starting up." << endl;

    Analyzer analyzer;
    Vec2 mapDims(512,512);
    analyzer.Load(mapDims,"data/st-helens/pre.data","data/st-helens/post.data");

    generateTestData(analyzer);

    cout << "Enter -1 to exit." << endl;
    cout << "Enter four integer values for x0 y0 x1 y1" << endl;

    Vec2 start;
    Vec2 end;
    while (start.x != -1) {
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
        float preDistance = analyzer.CalculatePathLength(analyzer.beforeData(),start,end);
        float postDistance = analyzer.CalculatePathLength(analyzer.afterData(),start,end);
        cout << "Before eruption: " << preDistance << endl;
        cout << "After eruption: " << postDistance << endl;
        cout << "Delta: " << postDistance - preDistance << endl;

        cout << "Calculating both at once." << endl;
        Vec2 pathLengths = analyzer.CalculatePathLengths(start, end);
        cout << "Before eruption: " << pathLengths.x << endl;
        cout << "After eruption: " << pathLengths.y << endl;
        cout << "Delta: " << pathLengths.y - pathLengths.x << endl;


    }

    cout << "Done." << endl;
    return 0;
}