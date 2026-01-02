# Ghost Hunt Simulation Game

Name: Noiva Chiu

## Program Description

This program runs a ghost hunting simulation, where hunters in a house attempt to identify the ghost present in it before running away due to boredom or fear. The ghosts spawns in a random room within the house, and the hunters are tasks with identifying evidence left behind by the ghost, starting in the van / exit room.
    
When the program is running, users are prompted to create the hunters by providing each hunter's name and ID until the user enters "done".

Additional functionality from the requirements has been included, in which the user may choose to select a specific device for each hunter. If no selection was made by the user, the program chooses a random device for the hunter.

## File Purposes

Provided Files

* main.c
    + main() function, manages main control flow
* defs.h
    + contains structure definitions and function declarations for data related to the house
* helpers.h
    + contains function declarations for helper functions implemented in helpers.c
* helpers.c
    + helper functions
* validate_logs.py
    + simulation logs validator

Created Files

* house.c
    + implements house related functions
* room.c
    + implements room related functions
* path.c
    + implements all functions managing a hunter's room path stack
* ghost.c
    + implements ghost related functions
* hunter.c
    + implements hunter related functions
* evidence.c
    + implements all functions handling room and case file evidence

* makefile
    + builds the program

## Building and Running Instructions

1. Open a terminal and navigate to the directory containing the program's files
2. Enter this command to compile and link files: make
3. Should see an executable file named 'project' appear
4. From the same directory, enter this command to execute the project: ./project
5. Once program starts running, follow prompts to create hunters and start the simulation
6. To validate the program logs, enter this command: python3 validate_logs.py
7. To remove object files, log files, and the executable file, enter this command: make clean

NOTE: It is necessary to remove all log files each time before running the program to ensure the validator works properly
