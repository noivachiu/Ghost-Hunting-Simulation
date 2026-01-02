#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

int run_test_functions(House *house);
int get_hunters(House *house);
void results_print(House *house);

int main() {

    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

    // Project with house allocated on stack
    House house;                                // house structure 
    int success;                                // flag for error checking

    success = house_create_stack(&house);      // initializes the house structure
    if (!success) {
        exit(0);
    }

    house_populate_rooms(&house);               // populates house structure with rooms

    success = house_load_data(&house);          // initializes ghost data, dynamic hunter array, case file, etc.
    if (!success) {
        exit(0);
    }

    success = get_hunters(&house);              // gets hunter data from user and appends hunters to house
    if (!success) {
        exit(0);
    }

    // Creates ghost thread
    pthread_create(&(house.ghost.thread), NULL, ghost_thread, &(house.ghost));     

    // Creates hunter threads
    for (int i = 0; i < house.hunter_arr.hunter_count; i++) {

        Hunter *hunter = house.hunter_arr.hunters[i];       // gets hunter pointer

        pthread_create(&(hunter->thread), NULL, hunter_thread, hunter);
    }

    // Waits for ghost thread to complete
    pthread_join(house.ghost.thread, NULL);

    // Waits for all hunter threads to complete
    for (int i = 0; i < house.hunter_arr.hunter_count; i++) {

        Hunter *hunter = house.hunter_arr.hunters[i];       // gets hunter pointer

        pthread_join(hunter->thread, NULL);
    }

    // RUN TEST FUNCTIONS
    // run_test_functions(&house);

    // Print results screen
    results_print(&house);

    house_cleanup_stack(&house);               // frees dynamically allocated memory for the house structure fields

    return 0;
}

// Gets hunter info from users
int get_hunters(House *house) {

    printf("\n===================== WILLOW HOUSE INVESTIGATION =====================\n");

    printf("\nPlease enter hunters one at a time.\n");
    printf("\nIf you would like to choose a specific device for each hunter, select one from below when prompted: \n");

    // Gets list of all evidence types
    const enum EvidenceType* device_types = NULL;
    int device_count = get_all_evidence_types(&device_types);

    // Prints all device options
    for (int i = 0; i < device_count; i++) {
        printf("     %d. %s\n", i, evidence_to_string(device_types[i]));
    }
    
    int c = C_OK;

    // Loops to ask user for input to create hunters until user enters "done" or error occurs
    do {
        c = hunter_user_create(house);
    } while (c == C_OK);

    return c;
}

// Prints results screen
void results_print(House *house) {

    printf("\n======================== INVESTIGATION  RESULTS ========================\n\n");

    // Prints all hunters simulation results
    hunters_all_result_print(&(house->hunter_arr));

    // Prints shared case file checklist
    casefile_results_print(&(house->case_file));

    bool hunters_win;       // tracks which entity won the game

    // Stores actual ghost type and hunter's guess ghost type
    enum GhostType ghost_actual = house->ghost.type;
    enum GhostType ghost_guess = (enum GhostType)house->case_file.collected;

    // Determines which entity won the game
    if (ghost_actual == ghost_guess) {
        hunters_win = true;
    }
    else {
        hunters_win = false;
    }

    // Prints victory results
    printf("\nVictory Results: \n");
    printf("--------------------------------------------------------------------\n");

    printf("    - Hunters exited after identifying the ghost: %d/%d \n", hunters_win_count(&(house->hunter_arr)), house->hunter_arr.hunter_count);

    printf("    - Ghost Guess: ");
    if (hunters_win) {
        printf("%s \n", ghost_to_string(ghost_guess));
    }
    else {
        printf("N/A \n");
    }

    printf("    - Actual Ghost Type: %s \n", ghost_to_string(ghost_actual));

    // Prints overall results
    printf("\nOverall Results: ");
    if (hunters_win) {
        printf("HUNTERS WIN!\n\n");
    }
    else {
        printf("GHOST WINS!\n\n");
    }
}

// Runs some data printing test functions
int run_test_functions(House *house) {

    printf("\nPRINTING HOUSE ROOMS...\n");
    house_print_rooms(house);

    printf("\nPRINTING GHOST IN HOUSE...\n");
    house_print_ghost(house);

    printf("\nPRINTING HUNTERS IN HOUSE...\n");
    house_print_hunters(house);

    printf("\n");

    return C_OK;
}
