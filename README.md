# MazeLock Simulation

## Introduction
MazeLock is a simulation program that creates a random maze-like matrix, representing a secure room. The program generates a new matrix periodically and tries to find a path from the entry point to the exit point in the matrix.

## Requirements
- GCC (GNU Compiler Collection)
- pthreads library (POSIX threads)

## Compilation
To compile the program, navigate to the directory containing the `mazelock.c` file and use the included `Makefile` to build the executable.

Run the following command in your terminal or command prompt:
make
This will generate an executable named `mazelock`.

## Running the program
After compiling the program, you can run it by executing the following command:
./mazelock


The program will prompt you for the number of rows, number of columns, and density of open cells in the matrix. Press 'Enter' to start the simulation.

During the simulation, you can press 'q' at any time to quit the program.

## Authors
- Ben Meddeb
- David Mcconnell

## License
This project is licensed under the MIT License -
