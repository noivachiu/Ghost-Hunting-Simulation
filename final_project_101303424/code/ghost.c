#include <stdio.h>
#include "defs.h"
#include "helpers.h"

// INITIALIZATION FUNCTIONS

/*
    Purpose: 
        Initializes a fields of a ghost structure.
    Parameters:
        - ghost (out): ghost structure
    Returns:
        C_ERR if error occurs, C_OK if successful.
*/
int ghost_init(Ghost *ghost) {

    if (ghost == NULL) {
        printf("\nERROR: Ghost pointer is NULL, cannot initialize ghost.\n");
        return C_ERR;
    }

    // Initializes ghost fields to simulation starting values
    ghost->id = DEFAULT_GHOST_ID;
    ghost->type = ghost_choose_rand_ghosttype();
    ghost->boredom = 0;
    ghost->running = true;
    ghost->exited = false;
    ghost->room = NULL;

    return C_OK;
}

/*
    Purpose:
        Randomly selects a ghost type for the ghost.
    Returns:
        Randomly selected ghost type;
*/
enum GhostType ghost_choose_rand_ghosttype() {

    // Gets ghost types list
    const enum GhostType* ghost_types = NULL;
    int ghost_count = get_all_ghost_types(&ghost_types);

    int rand_index = rand_int_threadsafe(0, ghost_count);       // generates random integer to choose ghost at that index

    return ghost_types[rand_index];
}

// GHOST THREADING FUNCTION
/*
    Purpose:
        Start function called when creating the ghost thread.
    Parameters:
        - args (in/out): pointer to a ghost structure
*/
void *ghost_thread(void *arg) {

    // Types casts the provided argument appropriately
    Ghost *ghost = (Ghost*)arg;

    // Ghost behaviour loop
    while (ghost->running) {
        
        ghost_take_turn(ghost);
    }

    return 0;
}

// GHOST TAKE TURN FUNCTION
/*
    Purpose:
        Executes one ghost simulation turn.
    Parameters:
        - ghost (in/out): ghost structure
*/
void ghost_take_turn(Ghost *ghost) {

    // Update ghost's stats
    bool can_move = ghost_stats_update(ghost);      // stores return value indicating if ghost can move

    // Checks if ghost should exit, ghost exits if true
    bool ghost_exited = ghost_condition_check(ghost);

    if (ghost_exited) {
        return;             // ends turn if ghost exited simulation
    }

    // Makes ghost take an action
    ghost_take_action(ghost, can_move);
}

// GHOST STATS FUNCTIONS

/*
    Purpose:
        Updates ghost's stats fields.
    Parameters:
        - ghost (in/out): ghost structure
    Returns: True if hunters are in the room (indicating ghost cannot move), false othewise.
*/
bool ghost_stats_update(Ghost *ghost) {

    // Waits for room hunter occupancy lock
    sem_wait(&(ghost->room->hunter_occupancy_lock));

    // Checks if there are hunters currently in room with ghost
    bool hunters_in_room = ghost_check_hunters(ghost->room);      
    
    // Releases room hunter occupancy lock
    sem_post(&(ghost->room->hunter_occupancy_lock));

    if (hunters_in_room) {
        ghost_boredom_reset(ghost);  
    } 
    else {
        ghost_boredom_inc(ghost);
    }

    return !hunters_in_room;
}

/*
    Purpose:
        Checks if there are hunters in the room ghost is currently in.
    Parameters:
        - room (in): room ghost is currently in
    Returns:
        True if there are hunters in the room, false otherwise.
*/
bool ghost_check_hunters(const Room *room) {

    if (room->hunter_arr.hunter_count > 0) {
        return true;
    }

    return false;
}

/*
    Purpose:
        Increases ghost's boredom.
    Parameters:
        - ghost (in/out): ghost structure
*/
void ghost_boredom_inc(Ghost *ghost) {

    (ghost->boredom)++;
}

/*
    Purpose:
       Resets ghost's boredom.
    Parameters:
        - ghost (in/out): ghost structure
*/
void ghost_boredom_reset(Ghost *ghost) {

    ghost->boredom = 0;
}

/*
    Purpose:
        Checks if ghosts should exit simulation, ghosts exits if true.
    Parameters:
        - ghost (in/out): ghost structure
    Returns:
        True if ghost has exited the simulation, false otherwise.
*/
bool ghost_condition_check(Ghost *ghost) {

    // Checks if ghost boredom level is equal to or above max
    if (ghost->boredom >= ENTITY_BOREDOM_MAX) {

        ghost_exit(ghost);           // exits ghost from simulation
        return true;
    }
    
    return false;
}

/*
    Purpose:
        Exits ghost from simulation.
    Parameters:
        - ghost (in/out): ghost structure
*/
void ghost_exit(Ghost *ghost) {

    Room *room = ghost->room;                                   // stores pointer to room ghost is exiting from for logs

    // Waits for room ghost presence lock
    sem_wait(&(room->ghost_presence_lock));

    room_remove_ghost(ghost->room, ghost);                      // removes ghost from room

    log_ghost_exit(ghost->id, ghost->boredom, room->name);       // logs ghost exiting the simulation

    // Releases room ghost presence lock
    sem_post(&(room->ghost_presence_lock));

    // Updates ghost simulation stat fields
    ghost->running = false;
    ghost->exited = true;
}

// BEHAVIOUR FUNCTIONS

/*
    Purpose:
        Randomly chooses action for ghost to take (depending on if it can move) and takes it.
    Parameters:
        - ghost (in/out): ghost structure
        - ghost_can_move (in): boolean value indicating if ghost can move
    Returns:
        C_OK 
*/
int ghost_take_action(Ghost *ghost, bool ghost_can_move) {

    // Creates array of function pointers to ghost's actions
    void (*ghost_actions[3])(Ghost*) = {ghost_idle, ghost_haunt, ghost_move};

    int rand_index;         // stores randomly generated integer to choose random action

    // Checks if ghost can move to determine range of actions ghost can take
    if (ghost_can_move) {
        rand_index = rand_int_threadsafe(0, 3);
    }
    else {
        rand_index = rand_int_threadsafe(0, 2);
    }

    // Calls randomly chosen ghost action function
    ghost_actions[rand_index](ghost);
    
    return C_OK;
}

/*
    Purpose:
        Executes action of the ghost idling, a placeholder for the act of doing nothing.
    Parameters:
        - ghost (in): ghost structure
*/
void ghost_idle(Ghost *ghost) {

    // Logs ghost's action
    log_ghost_idle(ghost->id, ghost->boredom, ghost->room->name);

    return; 
}

/*
    Purpose:
        Executes action of the ghost haunting, by randomly leaving a piece of evidence that can help identify the ghost in room.
    Parameters:
        - ghost (in): ghost structure
*/
void ghost_haunt(Ghost *ghost) {

    // Array to store evidence types that identify ghost
    enum EvidenceType ghost_evidence_types[3];
    ghost_to_evidence_types(ghost, ghost_evidence_types);                   // careful, not passing const ghost, may need to remove from function signatures

    // Randomly choose evidence for ghost to leave behind in room
    int rand_index = rand_int_threadsafe(0, 3);                             // generates random integer to choose evidence type at that index
    enum EvidenceType evidence_piece = ghost_evidence_types[rand_index];

    // Waits for room evidence lock
    sem_wait(&(ghost->room->evidence_lock));

    // Adds evidence to room
    room_evidence_add(ghost->room, evidence_piece);

    // Logs ghost's action
    log_ghost_evidence(ghost->id, ghost->boredom, ghost->room->name, evidence_piece);

    // Releases room evidence lock
    sem_post(&(ghost->room->evidence_lock));

    return;
}

/*
    Purpose:
        Executes action of ghost moving to next room.
    Parameters:
        - ghost (in/out): ghost structure
*/
void ghost_move(Ghost *ghost) {

    Room *current_room = ghost->room;                                   // stores pointer to ghost's current room for logs
    Room *next_room = room_choose_rand_connection(ghost->room);         // gets randomly chosen connected room for ghost to move to

    // will need to be careful about this function with multi-threading later on...need to follow entity room locking requirement

    // Waits for room ghost prescence locks (locks in order of memory addresses)
    if (current_room < next_room) {

        sem_wait(&(current_room->ghost_presence_lock));
        sem_wait(&(next_room->ghost_presence_lock));
    }
    else {
        sem_wait(&(next_room->ghost_presence_lock));
        sem_wait(&(current_room->ghost_presence_lock));
    }

    // Removes ghost from current room and adds ghost to next room
    room_remove_ghost(ghost->room, ghost);
    room_add_ghost(next_room, ghost);

    // Logs ghost's actions
    log_ghost_move(ghost->id, ghost->boredom, current_room->name, next_room->name);

    // Releases room ghost prescence locks 
    sem_post(&(current_room->ghost_presence_lock));
    sem_post(&(next_room->ghost_presence_lock));
}

// TESTING FUNCTIONS

// Prints ghost details
void ghost_print(const Ghost *ghost) {

    // If associated room pointer is NULL
    if (ghost->room == NULL) {
        printf("\nID: %-5d | Type: %-10s | Current Room: %-15s\n", ghost->id, ghost_to_string(ghost->type), "Unknown");
    }
    // If associated room pointer has been properly initialized
    else {
        printf("\nID: %-5d | Type: %-10s | Current Room: %-15s\n", ghost->id, ghost_to_string(ghost->type), ghost->room->name);
    }
    
    printf("Ghost type in byte form: ");
    print_bits((unsigned char)(ghost->type));
}