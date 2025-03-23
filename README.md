# element_cs

To run: 

Clone the repo.

Build the project with cmake (or use the CLion IDE with the included project, ensuring that the working directory in the target configurion is set to `$ProjectFileDir$` . 

<img width="603" alt="image" src="https://github.com/user-attachments/assets/67cfe5ac-dcd8-45a3-825c-8ff3f974d107" />

Run. 

Follow the prompts to enter coordinates for the line to be measured. 

# Approach

<img width="562" alt="image" src="https://github.com/user-attachments/assets/94919611-987d-42ce-9f0c-c144f3b5505e" />

In short: this version performs a 2D raycast against a mesh of two-triangle quads, calculating the intersection of the ray with each integer pixel boundary, as well as diagonals (from north-east to south-west) within
those quads. For every encountered intersection point, two height map values are sampled, based on the edge that was encountered. For intersections with a horizontal edge, sample the height map at `floor(dx),dy` and `ceil(dx),dy` . The distance from `dx,dy` to the adjacent edges is used as a lerp factor to interpolate between those two values from the height map to calculate the height at the floating point intersection position as a function of its neighbors. The same approached is used when encountering a vertical or diagonal intersection. When the sample falls precisely on a pixel, the logic effectively interpolates the pixel with itself, resulting in a wasteful NOP but it keeps the code a bit cleaner. 

A spatial distance is then calculated between this point `cx,cy,h` and the previous point. 
```
Vec2 intersection = ... ; // find intersection with raycast. 
float dx = interesection.x - prevIntersection.x;
float dy = intersection.y - prevItnersection.y;
float height =  sample(heightMap,dx,dy);
float dh = height - prevHeight;

// scale to real world units
dx *= 30; // meters
dy *= 30; 
dh *= 11;

float spatialDistance = sqrt(dx*dx + dy*dy + dh*dh);

pathLength += spatialDistance;
prevIntersection = intersection; 
prevHeight = height;
```

This illustration shows the intersection points (circles) and the edges that are sampled to interpolate the height at that position. 

<img width="515" alt="image" src="https://github.com/user-attachments/assets/a1ccd04f-6ee1-4124-95b7-cf1274acc7ab" />


Everything else in the project comes down to map loading, Vec2 data type, DDA raycast, etc. 


