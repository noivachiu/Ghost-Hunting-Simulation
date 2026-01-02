#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

/*
    Purpose:
       Initializes fields of a house structure stored on stack.
    Parameters:
        - house (out): house structure, owns and stores all simulation data
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int house_create_stack(House *house) {

    if (house == NULL) {
        printf("ERROR: House pointer is NULL, cannot initialize house.\n");
        return C_ERR;
    }

    // Initializes fields of house to simulation starting values
    CaseFile case_file = {0};
    Ghost ghost = {0};
    DynamicHunterArray hunters = {0};       

    house->case_file = case_file;
    house->ghost = ghost;
    house->hunter_arr = hunters;
    house->entities_running = false;

    house->starting_room = NULL;
    house->room_count = 0;

    return C_OK;
}

/*
    Purpose:
        Initalizes ghost, casefile structures and allocates them to the house structure.
        Adds ghost to a randomly chosen starting room.
    Parameters:
        - house (in/out): house structure
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int house_load_data(House *house) {

    int success;        // flag to track success of initializations

    casefile_init(&(house->case_file));                         // intializes case file structure
    ghost_init(&(house->ghost));                                // intializes ghost structure
    success = dynamic_hunterarr_init(&(house->hunter_arr));     // initializes dynamic hunter array structure

    if (!success) {
        return C_ERR;
    }

    Ghost *ghost = &(house->ghost);        // stores pointer to house's ghost

    // Adds ghost to random starting room
    Room* start_room = room_choose_rand_start(house);

    // Waits for room ghost presence lock
    sem_wait(&(start_room->ghost_presence_lock));

    room_add_ghost(start_room, ghost);

    // Releases room ghost prescence lock
    sem_post(&(start_room->ghost_presence_lock));

    // TESTING (spawns ghost in van to ensure ghost can detect hunters)
    // room_add_ghost(house->starting_room, ghost);

    // Logs ghost initialization
    log_ghost_init(ghost->id, ghost->room->name, ghost->type);

    // Tracks if there are running entities in the house (for single threading)
    if (ghost->running) {
        house->entities_running = true;
    }

    return C_OK;
}

/*
    Purpose:
        Adds hunter to house structure.
    Parameters:
        - house (in/out): house structure
        - hunter (in/out): hunter structure, to add to house
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int house_add_hunter(House *house, Hunter *hunter) {

    int success = dynamic_hunterarr_add(&(house->hunter_arr), hunter);      // adds hunter to dynamic hunter array

    if (!success) {
        return C_ERR;
    }

    hunter->case_file = &(house->case_file);        // points hunter's casefile to house's shared casefile

    // Permitted for hunters to point to exit without being added to the exit room's occupancy during initialization
    // Allows for more than 8 hunters to be added to house

    // Checks if there is room in the van 
    if (house->starting_room->hunter_arr.hunter_count < MAX_ROOM_OCCUPANCY) {

        hunter->init_added_to_van = true;
        room_add_hunter(house->starting_room, hunter);                  // adds hunter van's fixed hunter array       
    }
    // Adds van/exit room to hunter room path stack, without adding hunter to van's fixed hunter array
    else {

        hunter->room = house->starting_room;                            // points hunter's current room to the van/exit room
        roomstack_push(&(hunter->rooms_path), house->starting_room);    // adds van/exit room to hunter room path stack   
    }

    // Logs hunter initialization
    log_hunter_init(hunter->id, hunter->room->name, hunter->name, hunter->device_type);

    return C_OK;
}

// NOTE: only needed this function for single threading
/*
    Purpose:
        After all entities finish their turn, checks if there is at least one entity still running.
    Parameters:
        - house (in/out): house structure
*/
void house_check_entities_running(House *house) {

    bool entities_running = false;

    // Checks if ghost is running
    entities_running = entities_running || house->ghost.running;

    // Checks if hunters are running
    for (int i = 0; i < house->hunter_arr.hunter_count; i++) {
        entities_running = entities_running || house->hunter_arr.hunters[i]->running;
    }

    house->entities_running = entities_running;
}

/*
    Purpose:
        Frees the dynamic memory allocated for the house structure's dynamically allocated fields.
        Destroys all semaphores used for rooms and case file
    Parameters:
        - house (out): house structure
*/
void house_cleanup_stack(House *house) {

     // Checks if house pointer is NULL
    if (house == NULL) {

        printf("\nERROR: House pointer is NULL, cannot free allocated memory.\n");
        return;
    }

    dynamic_hunterarr_cleanup(&(house->hunter_arr));        // frees all memory dynamically allocated for dynamic hunter array

    int success;

    // Destroys all semaphores allocated to each room
    for (int i = 0; i < house->room_count; i++) {

        // Destroys ghost prescence lock semaphore
        success = sem_destroy(&(house->rooms[i].ghost_presence_lock));
        if (success != 0) {
            printf("\nERROR: Room lock semaphore could not be destroyed...\n");
            return;
        }

        // Destroys hunter occupancy lock semaphore
        success = sem_destroy(&(house->rooms[i].hunter_occupancy_lock));
        if (success != 0) {
            printf("\nERROR: Room lock semaphore could not be destroyed...\n");
            return;
        }

        // Destroys evidence lock semaphore
        success = sem_destroy(&(house->rooms[i].evidence_lock));
        if (success != 0) {
            printf("\nERROR: Room evidence lock semaphore could not be destroyed...\n");
            return;
        }
    }

    // Destroys case file semaphore
    success = sem_destroy(&(house->case_file.mutex));
    if (success != 0) {
        printf("\nERROR: Case file mutex could not be destroyed...\n");
    }

}

// TESTING FUNCTIONS

// Prints rooms in the house
void house_print_rooms(const House *house) {

    printf("\nNumber of rooms in house: %d \n", house->room_count);

    for (int i = 0; i < house->room_count; i++) {
        room_print(house->rooms + i);
    }
}

// Prints ghost in the house
void house_print_ghost(const House *house) {

    printf("\nGhost currently haunting house: \n");
    ghost_print(&(house->ghost));
}

// Prints all hunters in the house
void house_print_hunters(const House *house) {

    printf("\nNumber of hunters currently investigating the house: %d \n", house->hunter_arr.hunter_count);

    for (int i = 0; i < house->hunter_arr.hunter_count; i++) {
        hunter_print(house->hunter_arr.hunters[i]);
    }
}