# element_cs

To run: clone the repo

Build the project with cmake (or use the CLion IDE with the included project)

Run. 

Follow the prompts to enter coordinates for the line to be measured. 

## Alternate Approach

As they say, hindsight is 20/20. 

It occurred to me that all of the vertical boundary intersections are equally spaced in X and Y (based on the slope of the ray). The same goes for the horizontal and the diagonal intersections. (Where "intersection" indicates a point at which the height map must be sampled.) You would need an initial ray cast to find the first sample point of each of the three types (horizontal, vertical and diagonal), but after that, it's just addition. 

Rather than bothering with the raycast, one could just generate a list each for the horizontal intersections, vertical intersections and diagonal intersections. These lists (or queues) could be directly generated without the need for intersection testing at all. Then, in a loop, pick the closest non-visted intersection among the top of the three queues. Sample the height at that point, add the difference between that and the previous best height to the running total, and iterate until all three queues are empty. 

Duh. 

This approach would likely give a cleaner result than the implementation in this repository. Each intersection would be calculated using integer addition rather than floating point, where small errors can accumulate over many steps. No tricky diagonal intersections tests would be necessary after the first one, and the code would likely be much cleaner (and shorter). 

Queue "If I could turn back time..." by Cher.  Given another 3 hours to work on this, I'd definitely follow this approach, or at least throw a test together and see how well it works, or what unexpected challenges it would present. 

What follows is from my original submission. 

# Known Issues
Diagnoal calculation is off. I suspect an issue with sampling on diagonal edges. This is a solvable problem, but I wasn't able to correct the issue in the time alotted. 
Some possibilities include accumulated floating point error, incorrect interpolation on diagonals, or some basic math assumption I made that is incorrect.
Given the time, I'd write a comprehensive set of tests with known data and analytically calculable correct answers. For instance, 

* A simple ramp (heights from 0 to 255 in X
* All zeros. (I actually have a test for this, and it gives dx * 30, as expected, in horizontal and vertical. Diagonals yield a small discrepency from expected value (which is one reason I suspect diagonal sampling is a problem)
* All ones.  (Tested this; it gives the correct expected results.)
* Check existing data sets against known correct results

For those data sets, I'd test sampling only in X, only in Y and diagonals, and the sum of split segments. For instance, in the sample St. Helen's data, performing a sanity check by splitting the 0,0-511,511 diagnoal into two parts yields disappointing results:

| start | end | pre | post | delta |
| --- | --- | --- | --- | --- |
|0,0 | 255,255 | 11297.7 | 11085.6 | -212.1 |
|255,255 | 511,511 | 11472.5 | 11589.1 | 116.516 |
|0,0 | 511,511 | 22883.9 | 22928.4 | 44.5039 |

Obviously, -212.1 + 116.516 = -95.84, not the calculated 44.5039. 

Disabling diagnoal interpolation brings these values within 1% of each other. That's still a significant error that wouldn't be suitable for production, but, this could simply be an off-by-one error in raycast loop bounds. In either case, diagonal intersection detection and interpolation is necessary. I temporarily disabled it while testing to narrow down the source of the problem. 

I'd also like to incorporate a visualizer, or at at least mark visited cells in the color images and export a texture for analysis, so that I can see what's happening. While I started to put together a quick prototype, it would require more time than was alotted to complete. 

The image below illustrates the output of the modified DDA (raycast) algorithm across a flat mesh, with horizontal, vertical and diagonal intersection calculation. (This is from a sandbox app I built just for experimentation with things like this. Unfortunately, that system is incompatible with this project, but it illustrates that the traversal logic is sound.) 

*This indicates to me that the heigh sampling interpolation logic is incorrect.*

<img width="611" alt="image" src="https://github.com/user-attachments/assets/a25c955a-ae28-46e6-a61b-9ff3948e39b8" />







