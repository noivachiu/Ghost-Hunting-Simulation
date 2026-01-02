#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"   


// ROOM FUNCTIONS

/*
    Purpose:
       Initializes fields of a room structure.
    Parameters:
        - room (out):   room structure
        - name (in):    C-string room name
        - is_exit (in): indicates if room is exit
    Returns:
        C_ERR if error occurs, C_OK if successful.
*/
int room_init(Room* room, const char* name, bool is_exit) {

    if (room == NULL) {
        printf("\nERROR: Room pointer is NULL, cannot initialize room...\n");
        return C_ERR;
    }

    // Initializes field of room with provided parameters
    strcpy(room->name, name);       
    room->is_exit = is_exit;

    // Initializes semaphores
    if ((sem_init(&(room->ghost_presence_lock), 0, 1)) < 0) {
        printf("\nERROR: On semaphore init...\n");
        exit(1);
    }

    // Initializes semaphores
    if ((sem_init(&(room->hunter_occupancy_lock), 0, 1)) < 0) {
        printf("\nERROR: On semaphore init...\n");
        exit(1);
    }

    if ((sem_init(&(room->evidence_lock), 0, 1)) < 0) {
        printf("\nERROR: On semaphore init...\n");
        exit(1);
    }

    // Initialize other fields of room to simulation starting values
    room->evidence = 0;             
    room->connect_count = 0;
    room->ghost = NULL;

    fixed_hunterarr_init(&(room->hunter_arr));      // initializes fixed hunter array

    return C_OK;
}

/*
    Purpose:
        Connects rooms adjacent to each other.
    Paremeters:
        - a (in/out): room structure a
        - b (in/out): room structure b
    Returns:
        C_ERR if error occurs, C_OK if successful.
*/
int room_connect(Room* a, Room* b) {

    if ((a == NULL) || (b == NULL)) {
        printf("\nERROR: Room A or B pointers are NULL, cannot connect...\n");
        return C_ERR;
    }

    // Checks if either rooms a or b have reached max room connections
    if ((a->connect_count == MAX_CONNECTIONS) || (b->connect_count == MAX_CONNECTIONS)) {
        printf("\nERROR: Rooms A or B have already reached max connections, cannot connect...\n");
        return C_ERR;
    }

    // Connects rooms
    a->rooms_connected[a->connect_count] = b;
    b->rooms_connected[b->connect_count] = a;

    // Increments rooms connected counter
    (a->connect_count)++;
    (b->connect_count)++;

    return C_OK;
}

/*
    Purpose:
        Randomly selects a starting room for the ghost to initially spawn in.
    Parameters:
        - house (in):
    Returns:
        Pointer to a randomly selected room in the house.
*/
Room* room_choose_rand_start(House *house) {

    int rand_index = rand_int_threadsafe(0, house->room_count);     // get random index by generating random integer 
    
    return house->rooms + rand_index;       
}

/*
    Purpose:
        Randomly selects a connected room for an entity to move to.
    Parameters:
        - room (in): room structure
    Returns:
        Pointer to a randomly selected room connected to the provided room.
*/
Room* room_choose_rand_connection(Room *room) {

    int rand_index = rand_int_threadsafe(0, room->connect_count);    // get random index by generating random integer 
    
    return room->rooms_connected[rand_index];       
}

/*
    Purpose: 
        Adds ghost to provided room.
    Parameters:
        - room (out): room structure, to add ghost
        - ghost (out): ghost structure, to add to room
*/
void room_add_ghost(Room *room, Ghost *ghost) {

    ghost->room = room;
    room->ghost = ghost;
}

/*
    Purpose: 
        Removes ghost from provided room.
    Parameters:
        - room (out): room structure, to remove ghost
        - ghost (out): ghost structure, to remove from room
*/
void room_remove_ghost(Room *room, Ghost *ghost) {

    ghost->room = NULL;
    room->ghost = NULL;
}

/*
    Purpose: 
        Attempts to add hunter to provided room.
    Parameters:
        - room (in/out): room structure, to add hunter
        - hunter (in/out): hunter structure, to add to room
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int room_add_hunter(Room *room, Hunter *hunter) {

    // Checks if room has reached max occupancy (this is a double check)
    if (room->hunter_arr.hunter_count >= MAX_ROOM_OCCUPANCY) {
        printf("\nERROR: Room occupancy is at max, cannot add hunter to room...\n");
        return C_ERR;
    }

    // Adds hunter to room's fixed hunter array
    fixed_hunterarr_add(&(room->hunter_arr), hunter);

    hunter->room = room;                                              // updates hunter's current room pointer

    // If hunter is returning to exit, do not push room to room path stack
    if (hunter->return_to_van) {
        return C_OK;
    }

    int success = roomstack_push(&(hunter->rooms_path), room);        // pushes room to hunter room path stack

    return success;
}

/*
    Purpose: 
        Removes hunter from provided room.
    Parameters:
        - room (in/out): room structure, to remove ghost
        - hunter (in/out): ghost structure, to remove from room
    Returns:
        C_OK if successful, C_NOT_FOUND if hunter not found in room.
*/
int room_remove_hunter(Room *room, Hunter *hunter) {

    // Checks if hunter has just been initialized and was unable to be added to van
    if (!(hunter->init_added_to_van)) {

        hunter->init_added_to_van = true;       // hunters can now be removed from fixed hunter arrays after being properly added to other rooms
    }
    // Removes hunter from room's fixed hunter array
    else {
        int success = fixed_hunterarr_remove(&(room->hunter_arr), hunter);

        if (!success) {
            return C_ERR;
        }
    }

    hunter->room = NULL;                                // updates hunter's current room pointer

    // Checks if hunter is returning to van/exit room and is not already in the exit room
    if ((hunter->return_to_van) && (!(hunter_exit_check(room)))) {
        roomstack_pop(&(hunter->rooms_path));           // pops room froom hunter room path stack
    }
    
    return C_OK;
}

/*
    Purpose:
        Adds evidence that ghost leaves behind to room evidence.
    Parameters:
        - room (in/out): room structure, to add evidence
        - evidence (in): evidence, left by ghost
*/
void room_evidence_add(Room *room, enum EvidenceType evidence) {

    room->evidence = evidence_byte_set_type(room->evidence, evidence);       // combines ghost dropped evidence with room's evidence byte
}    

/*
    Purpose:
        Clears evidence that hunters has identified from room.
    Parameters:
        - room (in/out): room structure, to clear evidence
        - hunter (in): hunter structure, that identified the evidence
*/
void room_evidence_clear(Room *room, enum EvidenceType evidence) {

    room->evidence = evidence_byte_clear_type(room->evidence, evidence);    // clears hunter identified evidence in room's evidence byte 
}


// TESTING FUNCTIONS

// Prints details of room structure
void room_print(const Room *room) {

    printf("\nROOM NAME: %-25s   | ROOM EXIT: %d \n", room->name, room->is_exit);
    printf("Connected Rooms: \n");

    for (int i = 0; i < room->connect_count; i++) {
        printf("    %d. %s \n", i, room->rooms_connected[i]->name);
    }

    printf("Room Evidence Byte: ");
    print_bits((unsigned char)(room->evidence));
}
