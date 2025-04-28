
The code files are Exact.cpp and CoreExact.cpp.

The website is hosted using index.html.

You want it cleaned up and consistent, matching your project's context (not clique detection anymore).

Here’s the edited README for your project:

Exact Algorithms for Maximum Independent Set
Overview
This project implements and analyzes exact algorithms for solving the Maximum Independent Set (MIS) problem on graphs. The focus is on two approaches:

A baseline backtracking algorithm (Exact.cpp)

An optimized algorithm using Core Reduction techniques (CoreExact.cpp)

Additionally, a project webpage has been developed using index.html to present the algorithms, methodology, and results.

Execution Instructions
Compile the Algorithms
Use the following command to compile the C++ files. Replace <filename> with either Exact or CoreExact depending on which algorithm you want to run:

g++ -O3 <filename>.cpp -o <output_filename>.out
Note: The -O3 flag enables maximum optimization for faster execution.

Run the Compiled Program
Execute the compiled binary:

./<output_filename>.out
You will be prompted to enter the path to the input graph file. Ensure the file path is correct relative to the directory where the code is located to avoid errors like "file not found" or segmentation faults.

