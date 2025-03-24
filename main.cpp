#include <iostream>

using namespace std;

#include "Analyzer.h"

bool inRange(const Vec2& vec, double mn, double mx) {
    return vec.x >= mn && vec.x <= mx && vec.y >= mn && vec.y <= mx;
}

void runTests(const Vec2& mapDims, const vector<unsigned char>& beforeData, const vector<unsigned char>& afterData) {
    const vector<unsigned char> zeros(mapDims.x * mapDims.y, 0);
    auto& testData = zeros;
    //auto& testData = beforeData;

    cout << "START TEST RUN" << endl;

    vector<vector<Vec2>> sdTests
    {
        { Vec2(0,0), Vec2{255,255}, Vec2(511,511) },
        { Vec2(0,0), Vec2(255,63), Vec2(510,126) },
        { Vec2 (0,0), Vec2(255,0), Vec2(511,0) }
    };

    cout << "Split diagonal tests" << endl;
    for (auto& v : sdTests)
    {
        Vec2& p0 = v[0];
        Vec2& p1 = v[1];
        Vec2& p2 = v[2];

        const char* formatStr = "Dist\t %.2f, %.2f\t - \t%.2f, %.2f = \t\t%.2f\n";
        double d01 = Analyzer::CalculatePathLength(testData, mapDims, p0,p1);
        printf(formatStr, p0.x, p0.y, p1.x, p1.y, d01);
        double d12 = Analyzer::CalculatePathLength(testData, mapDims, p1,p2);
        printf(formatStr, p1.x, p1.y, p2.x, p2.y, d12);
        double d02 = Analyzer::CalculatePathLength(testData,mapDims,p0,p2);
        printf(formatStr, p0.x, p0.y, p2.x, p2.y, d02);

        double error = d02 - d01 - d12;
        printf("error = %.4f\n\n",error);
    }

    cout << "************" << endl;
    {
        Vec2 start(0,0);
        Vec2 end(511,0);

        printf("Horizontal test from %.2f, %.2f to %.2f, %.2f\n", start.x, start.y, end.x, end.y);

        double cp0 = 0;
        double cp1 = 0;
        for (int i=start.x + 1; i <= end.x; ++i) {
            double d0 = 11.0f * abs(beforeData[i] - beforeData[i-1]);
            cp0 += sqrt(30*30+d0*d0);
            double d1 = 11.0f * abs(afterData[i] - afterData[i-1]);
            cp1 += sqrt(30*30 + d1*d1);
        }

        cout << "From simple loop: " << cp0 << " after " << cp1 << " delta " << (cp1 - cp0) << endl;
        double ap0 = Analyzer::CalculatePathLength(beforeData,mapDims,start, end);
        double ap1 = Analyzer::CalculatePathLength(afterData,mapDims,start,end);
        cout << "From Analyzer:\n  Before: " << ap0 << " after " << ap1 << " delta " << ap1 - ap0 << endl;

        cout << "Before\tAfter\tDelta\n";
        cout << cp0 << "\t" << cp1 << "\t" << (cp0 - cp1) << endl;
        cout << ap0 << "\t" << ap1 << "\t" << (ap0 - ap1) << endl;
        cout << "-----------------------------------------------------" << endl;
        cout << (cp0 - ap0) << "\t" << (cp1 - ap1) << "\t" << (cp1 - cp0) - (ap1 - ap0) << endl;
    }

    cout << "END TEST RUN" << endl << endl << endl;
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
        /*
        A more efficient approach would be to calculate path lengths from both buffers simultaneously, in a single pass, since
        the sample locations are a function of line start and end positions, which will be the same for both height maps.    
        For a 512x512 map, the execution time difference is negligible. For larger data sets, or if we needed to run this 
        method many times in a loop, then it would be worthwhile to add a specialized version of the CalculatePathLength function
        to handle both maps at once. 
        */
        double preDistance = Analyzer::CalculatePathLength(beforeData, mapDims, start,end);
        double postDistance = Analyzer::CalculatePathLength(afterData, mapDims,start,end);
        cout << "Before eruption: " << preDistance << endl;
        cout << "After eruption: " << postDistance << endl;
        cout << "Delta: " << postDistance - preDistance << endl;
    }

    cout << "Done." << endl;
    return 0;
}
