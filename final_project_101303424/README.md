# Final Project - Simulating the Hunt

## Student Information

    Name: Noiva Chiu
    Student ID #: 101303424
    Course: COMP2401 A

## Program Description

    This program runs a ghost hunting simulation, where hunters in a house attempt to identify the ghost present in it before running away due to boredom or fear. The ghosts spawns in a random room within the house, and the hunters are tasks with identifying evidence left behind by the ghost, starting in the van / exit room.
    
    When the program is running, users are prompted to create the hunters by providing each hunter's name and ID until the user enters "done".

    Additional functionality from the requirements has been included, in which the user may choose to select a specific device for each hunter. If no selection was made by the user, the program chooses a random device for the hunter.

## File Purposes

* defs.h
    + header file containing all function declarations relating to all data stored in the house

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

My Input Testing Files

* hunters_inputs_random.txt
    + test inputs to feed into program (hunters have randomly assigned devices)
* hunters_inputs_strategic.txt
    + test inputs to feed into program (hunters have been assigned each type of device)
* hunters_inputs_large_set.txt
    + test inputs to feed into program (creates 50 hunters)

## Building and Running Instructions

1. Open a terminal and navigate to the directory containing the program's files
2. Enter this command to compile and link files: make
3. Should see an executable file named 'project' appear
4. From the same directory, enter this command to execute the project: ./project

To test the program for memory leaks with valgrind, do the following:
4. From the same directory, enter this command to execute the project through valgrind, enabling leak checker: valgrind --leak-check=full ./project

To test the program for race conditions with -fsanitize=thread, do the following:
2. Enter this command to compile and link files: gcc -Wall -Wextra -g -fsanitize=thread -lpthread main.c house.c ghost.c hunter.c room.c evidence.c path.c helpers.c -o project
3. Should see an executable file named 'project' appear
4. From the same directory, enter this command to execute the project: ./project

5. Once program starts running, follow prompts to create hunters and start the simulation

6. To validate the program logs, enter this command: python3 validate_logs.py
7. To remove object files, log files, and the executable file, enter this command: make clean

NOTE:
* It is necessary to remove all log files each time before running the program to ensure the validator works properly

## Bonuses Included

* Documentation

## Sources Used

    Title: Getting input from users
    Author: Noiva Chiu
	Date: November 20, 2025
	Type: Source Code
	Availability: My COMP2401 Assignment A2

    Title: Bitwise operators
    Author: Noiva Chiu
	Date: November 20, 2025
	Type: Source Code
	Availability: My COMP2401 Assignment A3

    Title: Dynamically allocating memory 
    Author: Noiva Chiu
	Date: November 20, 2025
	Type: Source Code
	Availability: My COMP2401 Assignment A4

    Title: Handling deadlocks
    Author: C. Hillen
    Date: November 29, 2025
    Type: Explanations & examples
    Availability: Lecture 10.2 - Threading (slides 22-23)

    Title: Dynamically reallocating memory
    Author: M. Lanthier
	Date: November 23, 2025
	Type: Source Code
	Availability: COMP2401 Course Notes (p.g. 130-131)

    Title: Function pointers
    Author: M. Lanthier
	Date: November 23, 2025
	Type: Source Code
	Availability: COMP2401 Course Notes (p.g. 160-163)

    Title: Threading
    Author: M. Lanthier
	Date: November 27, 2025
	Type: Source Code
	Availability: COMP2401 Course Notes (p.g. 223-225)

    Title: Semaphores/mutex
    Author: M. Lanthier
	Date: November 27, 2025
	Type: Source Code
	Availability: COMP2401 Course Notes (p.g. 230-232)

    Title: Where to call logging functions
    Peer: Bhuvan Mohan
    Date: November 30, 2024
    Type: Discussion

## Assumptions Made Regarding Implementation

R-3
* Assuming that a hunter can only return to the exit room and exit the simulation due to evidence if:   
    + Case file contains 3 unique pieces of evidence
    + Hunter themselves have either identified evidence themselves or by chance were made to return to the exit room
    + Hunter did not exit the simulation along the way due to boredom or fear
+ Meaning that if one hunter identifies a victory, it does not mean all remaining hunters in the simulation will return to the exit room and exit the simulation due to evidence.

R-7.2
* Assuming that the first hunter to identify a victory marks that case file as solved

R-18 and R-19
* Ordering of behaviours is vague, project overview video I/O suggests that a hunter should check if its boredom/fear has reached max and exit if true before checking if room hunter is currently in is exit room, while specifications seem to suggest the opposite.
* Assuming I should follow the order suggested in project overview video I/O
    + Means that if hunter was on its way to returning to van and has managed to reach the van, but the hunter's boredom/fear has reached the max, it will exit the simulation because of fear/boredom, without making a complete exit room check (only to log that it has finished its return to the van)
