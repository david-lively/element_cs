# element_cs

To run: clone the repo

Build the project with cmake (or use the CLion IDE with the included project)

Run. 

Follow the prompts to enter coordinates for the line to be measured. 

# Known Issues
Diagnoal calculation is off. I suspect an issue with sampling on diagonal edges. This is a solvable problem, but I wasn't able to correct the issue in the time alotted. 
Some possibilities include accumulated floating point error, incorrect interpolation on diagonals, or some basic math assumption I made that is incorrect.
Given the time, I'd write a comprehensive set of tests with known data and analytically calculable correct answers. For instance, 

* A simple ramp (heights from 0 to 255 in X. I'd expect a result of
* All zeros. (I actually have a test for this, and it gives dx * 30, as expected, in horizontal and vertical. Diagonals yield a small discrepency from expected value (which is one reason I suspect diagonal sampling is a problem)
* All ones.

For those data sets, I'd test sampling only in X, only in Y and diagonals, and the sum of split segments. For instance, in the sample St. Helen's data, performing a sanity check by splitting the 0,0-511,511 diagnoal into two parts yields disappointing results:

| start | end | pre | post | delta |
| --- | --- | --- | --- | --- |
|0,0 | 255,255 | 11297.7 | 11085.6 | -212.1 |
|255,255 | 511,511 | 11472.5 | 11589.1 | 116.516 |
|0,0 | 511,511 | 22883.9 | 22928.4 | 44.5039 |

Obviously, -212.1 + 116.516 = -95.84, not the calculated 44.5039. 

I'd also like to incorporate a visualizer, or at mark visited cells in the color images and export a texture for analysis, so that I can see what's happening. While I started to put together a quick prototype, it would require more time than was alotted to complete. 

