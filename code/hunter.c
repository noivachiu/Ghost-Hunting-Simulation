#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

// USER CREATE HUNTER FUNCTION

/*
    Purpose:
        Gets fields from user to create a hunter, initializes hunter and adds it to house dynamic hunter array.
    Parameters:
        house (in/out): house structure, storing a pointer to a dynamic array containing a pointer to hunter
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int hunter_user_create(House *house) {

    Hunter *new_hunter;     // declares pointer to store pointer to user created hunter
    int success;            // flag to track success of hunter creation

    // Asks user for hunter's name (or done if finished creating hunters)
    char name[MAX_HUNTER_NAME];
    printf("\nEnter hunter name (max 63 chars) or \"done\" if finished: ");
    get_str(name);                                                  // gets user input for hunter name
    printf("\n");

    // Checks if user is done creating hunters
    if (strcmp(name, "done") == 0) {
        return C_DONE;
    }

    // Asks user for hunter's ID value
    int id;
    printf("Enter hunter's ID (integer): ");
    get_int(&id);
    printf("\n");

    // Asks user if they want to select a specific device for hunter
    char choose_device[MAX_INPUT_STRING];
    printf("Do you want to select a device? (\"yes\" or \"no\"): ");
    get_str(choose_device);
    printf("\n");

    // If user wants to select a specific device
    if (strcmp(choose_device, "yes") == 0) {

        // Asks user to choose device type based on index
        int device_index;
        printf("Select device based on index (integer between 0-6): ");
        get_int(&device_index);
        printf("\n");

        success = hunter_init(&new_hunter, name, id, true, device_index);
    }
    // Randomly chooses device for hunter
    else {
        success = hunter_init(&new_hunter, name, id, false, -1);            
    }

    if (!success) {
        return C_ERR;
    }

    // Adds hunter to house structure
    success = house_add_hunter(house, new_hunter);

    if (!success) {
        return C_ERR;
    }

    return C_OK;
}

// Reused from A2
/* 
    Purpose: 
        Gets string input from user, taking care of leftover characters in input buffer if too many were inputted by user.
    Parameters:
        - output_str (in/out): user's inputted string
    Returns: 
        C_OK.
*/
void get_str(char *output_str) {

    char str[MAX_INPUT_STRING];
    fgets(str, MAX_INPUT_STRING, stdin);

    // Checks if user inputted exact amount/more chars than allowed by buffer
    // If char value at index of last string char is not '\n', means there are leftover chars in input buffer to be cleaned up
    if (str[strnlen(str, sizeof(str)) - 1] != '\n') {   
        printf("Character overflow, clearing input buffer...\n"); 
        while (getchar() != '\n');   // Clears input buffer
    }
    else {
        str[strnlen(str, sizeof(str)) - 1] = '\0';  // Replaces newline '\n' with null-terminator '\0' if found
    }

    strcpy(output_str, str);    // Copies value of user input onto output string
}

// Reused from A2
/*
    Purpose:
        Gets integer input from user, taking care of leftover characters in input buffer.
    Parameters:
        - output_int (out): user's inputted integer
*/
void get_int(int *output_int) {
    
    scanf("%d", output_int);
    while (getchar() != '\n');
}

// HUNTER INITIALIZATION & CLEANUP FUNCTIONS

// Reused and modified from A4
/*
    Purpose:    
        Dynamically allocates a hunter structure and initializes its fields.
    Parameters:
        - hunter (out): hunter structure
        - name (in): C-string hunter name
        - id (in): hunter ID value
        - chose_device (in): boolean indicating if user chose device
        - device_index (in): index of device chosen by user, -1 if not chosen
    Returns:
        C_OK if succesful, C_ERR otherwise.
*/
int hunter_init(Hunter* *hunter, const char* name, const int id, const bool chose_device, const int device_index) {

    *hunter = (Hunter*) malloc(sizeof(Hunter));    // dynamically allocates a hunter structure

    if (*hunter == NULL) {
        printf("\nERROR: Memory allocation error... \n");
        return C_ERR;
    }

    // Initializes field of hunter with provided parameters
    strcpy((*hunter)->name, name);                             
    (*hunter)->id = id;
    (*hunter)->device_type = hunter_choose_device(chose_device, device_index);

    // Initializes fields of hunter to simulation starting values
    (*hunter)->boredom = 0;
    (*hunter)->fear = 0;
    (*hunter)->case_file = NULL;
    (*hunter)->room = NULL;
    (*hunter)->init_first_room = true;
    (*hunter)->init_added_to_van = false;
    (*hunter)->running = true;
    (*hunter)->return_to_van = false;
    (*hunter)->exited = false;
    (*hunter)->exited_reason = LR_NOT_YET_EXIT;         // unsure this is necessary

    roomstack_init(&((*hunter)->rooms_path));

    return C_OK;
}

// Reused and modified from A4
/*
    Purpose:
        Frees the memory allocated for the hunter structure.
    Parameters:
        - hunter (out): hunter structure
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int hunter_cleanup(Hunter* *hunter) {

    if (hunter == NULL) {
        printf("\nERROR: Hunter double pointer is NULL, cannot free allocated memory.\n");
        return C_ERR;
    }
    else if (*hunter == NULL) {
        printf("\nERROR: Hunter pointer is NULL, cannot free allocated memory.\n");
        return C_ERR;
    }

    free(*hunter);       // frees memory allocated for hunter structure
    *hunter = NULL;      // sets pointer pointed to by hunter to NULL

    return C_OK;
}

/*
    Purpose:
        Chooses device for hunter based on specified or random choice.
    Parameters:
        - chose_device (in): boolean indicating if specific device is chosen
        - device_index (in): index of chosen device, -1 if not chosen
    Returns:    
        Specified or randomly chosen device.
*/
enum EvidenceType hunter_choose_device(const bool chose_device, const int device_index) {

    // Gets list of all evidence types
    const enum EvidenceType* device_types = NULL;
    int device_count = get_all_evidence_types(&device_types);

    // If specific device chosen, returns chosen device
    if (chose_device) {

        // Checks if provided device index is within valid range
        if ((device_index >= 0) && (device_index < 7)) {
            return device_types[device_index];
        }
    }
    // If device not chosen or provided device index is out of range, returns randomly chosen device
    int rand_index = rand_int_threadsafe(0, device_count);       // generates random integer to choose device at that index
    return device_types[rand_index];
    
}

// HUNTER THREADING FUNCTION
/*
    Purpose:
        Start function called when creating a hunter thread.
    Parameters:
        - args (in/out): pointer to a hunter structure
*/
void *hunter_thread(void *arg) {

    Hunter *hunter = (Hunter*)arg;

    while (hunter->running) {
        
        hunter_take_turn(hunter);
    }

    return 0;
}

// HUNTER TAKE TURN FUNCTION
/*
    Purpose:
        Executes one hunter simulation turn.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_take_turn(Hunter *hunter) {

    bool hunter_exited;         // tracks if hunter has exited the simulation

    // Updates hunter's stats
    hunter_stats_update(hunter);

    // Checks if hunters has reached the exit room to log hunter's successful return
    if (hunter_exit_check(hunter->room)) {
        
        // Checks if hunter was returning to exit room
        if (hunter->return_to_van) {

            hunter->return_to_van = false;      // marks that hunter has reached the van

            // Logs hunter's end of returning to exit room
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->room->name, hunter->device_type, hunter->return_to_van);
        }
    }

    // Checks if hunter should exit, hunter exits if true
    hunter_exited = hunter_condition_check(hunter);

    if (hunter_exited) {
        return;             // ends turn if hunter exited simulation
    }

    // Checks if hunter is in exit room, manages exit room if true
    if (hunter_exit_check(hunter->room)) {
        hunter_exited = hunter_manage_exit_room(hunter);        // need to track if hunter exited
    }

    if (hunter_exited) {
        return;             // ends turn if hunter exited simulation
    }

    // Checks if hunter is on its way to exit room, if so does not gather evidence
    // Makes no sense to gather evidence with same device when already evidence identified
    if (!hunter->return_to_van) {

        // Makes hunter attempt to gather evidence
        hunter_gather_evidence(hunter);
    }

    // Makes hunter attempt to move
    hunter_move(hunter);
}

// HUNTER STATS FUNCTIONS

/*
    Purpose:
        Updates hunter's stats fields.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_stats_update(Hunter *hunter) {

    // Waits for room ghost prescence lock
    sem_wait(&(hunter->room->ghost_presence_lock));

    // Checks if ghost is currently in room with hunter
    bool ghost_in_room = hunter_check_ghost(hunter->room);

    // Releases room ghost prescence lock
    sem_post(&(hunter->room->ghost_presence_lock));

    if (ghost_in_room) {
        hunter_boredom_reset(hunter);
        hunter_fear_inc(hunter);
    }
    else {
        hunter_boredom_inc(hunter);
    }
}

/*
    Purpose:
        Checks if there ghost is in the room hunter is currently in.
    Parameters:
        - room (in): room hunter is currently in
    Returns:
        True if ghost is in the room, false otherwise.
*/
bool hunter_check_ghost(const Room *room) {

    if (room->ghost != NULL) {
        return true;
    }

    return false;
}

/*
    Purpose:
        Increases hunters's boredom.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_boredom_inc(Hunter *hunter) {

    (hunter->boredom)++;
}

/*
    Purpose:
       Resets hunters's boredom.
    Parameters:
        - hunter (out): hunter structure
*/
void hunter_boredom_reset(Hunter *hunter) {

    hunter->boredom = 0;
}

/*
    Purpose:
       Increases hunters's fear.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_fear_inc(Hunter *hunter) {

    (hunter->fear)++;
}

/*
    Purpose:
        Checks if hunter should exit simulation, hunter exits if true.
    Parameters:
        - hunter (in/out): hunter structure
    Returns;
        True if hunter has exited the simulation, false otherwise.
*/
bool hunter_condition_check(Hunter *hunter) {

    bool exited = false;        // flag to track if hunter has exited

    // Checks if hunter boredom level is equal or above max
    if (hunter->boredom >= ENTITY_BOREDOM_MAX) {

        hunter_exit(hunter, LR_BORED);          // exits hunter from simulation
        exited = true;
    }
    // Checks if hunter fear level is equal or above max
    else if (hunter->fear >= HUNTER_FEAR_MAX) {

        hunter_exit(hunter, LR_AFRAID);         // exits hunter from simulation
        exited = true;
    }

    return exited;
}

/*
    Purpose:
        Exits hunter from simulation.
    Parameters:
        - hunter (in/out): hunter structure
        - exit_reason (in): hunter exit reason
*/
void hunter_exit(Hunter *hunter, enum LogReason exit_reason) {

    Room *room = hunter->room;          // stores pointer to room hunter is exiting from for logs
    
    // Updates hunter's exit reason
    hunter->exited_reason = exit_reason;

    // Waits for room hunter occupancy lock
    sem_wait(&(room->hunter_occupancy_lock));

    room_remove_hunter(hunter->room, hunter);       // removes hunter from room

    log_exit(hunter->id, hunter->boredom, hunter->fear, room->name, hunter->device_type, hunter->exited_reason);        // logs hunter exiting the simulation

    // Releases room hunter occupancy lock
    sem_post(&(room->hunter_occupancy_lock));

    // Updates hunter simulation stats fields
    hunter->running = false;
    hunter->exited = true;

    roomstack_cleanup(&(hunter)->rooms_path, true);   // frees memory allocated for hunter's room path stack
}

// HUNTER BEHAVIOUR FUNCTIONS

/*
    Purpose:
        Checks if hunter is currently in the van/exit room.
    Parameters:
        - room (in): room hunter is currently in.
    Returns:
        True if provided room is the exit room, false otherwise.
*/
bool hunter_exit_check(const Room *room) {

    return room->is_exit;
}

/*
    Purpose:
        Manages exit room tasks hunter must do.
    Parameters:
        - hunter (in/out): hunter structure
    Returns:
        True if hunter has exited the simulation, false otherwise.
*/
bool hunter_manage_exit_room(Hunter *hunter) {

    // Waits for case file mutex
    sem_wait(&(hunter->case_file->mutex));

    // Checks for victory (3 pieces of evidence shared among hunters)
    bool victory =  casefile_check_victory(hunter->case_file);

    // If hunter finds that the shared case file identifies a valid ghost
    if (victory) {

        // Checks if hunter is the first to identify a victory
        if (!hunter->case_file->solved) {
            casefile_solved(hunter->case_file);
        }

        // Releases case file mutex (here if victory)
        sem_post(&(hunter->case_file->mutex));

        hunter_exit(hunter, LR_EVIDENCE);       // exits hunter from simulation
        return true;
    }

    // Releases case file mutex (here if not victory)
    sem_post(&(hunter->case_file->mutex));

    // Checks if hunter is still in exit room after being initialized, hunter should not swap devices or clear room path stack
    if (hunter->init_first_room) {
        return false;
    }

    // Swaps hunter's device
    hunter_swap_device(hunter);

    // Clears hunter's room path stack
    roomstack_cleanup(&(hunter->rooms_path), false);

    return false;
}

/*
    Purpose:
        Swaps hunters device based on random choice.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_swap_device(Hunter *hunter) {

    // May want to implement a smarter strategy in the future

    enum EvidenceType current_device = hunter->device_type;             // stores hunter's current device to be swapped for logs    
    enum EvidenceType new_device = hunter_choose_device(false, -1);     // randomly chooses new device for hunter (may end up with the same device)

    hunter->device_type = new_device;       // assigns (potentially) new device to hunter

    // Logs hunter's device swap
    log_swap(hunter->id, hunter->boredom, hunter->fear, current_device, new_device);
}

/*
    Purpose:
        Manages hunter's attempt to gather evidence in room.
    Parameters:
        - hunter (in/out): hunter structure
*/
void hunter_gather_evidence(Hunter *hunter) {

    // Waits for room evidence lock
    sem_wait(&(hunter->room->evidence_lock));

    // Checks if hunter's device matches any evidence present in room
    if (!hunter_check_evidence(hunter)) {

        // Releases room evidence lock (here when no matching evidence identified)
        sem_post(&(hunter->room->evidence_lock));

        // Checks if hunter is currently in exit room
        if (hunter_exit_check(hunter->room)) {

            return;                             // do not want to provide chance for hunter to return to exit room when already in it
        }                                       

        hunter_return_exit(hunter, false);      // provides a small chance for hunter to return to exit room         

        return;                                 // returns as no evidence was found
    }

    // Logs hunter's identified evidence
    log_evidence(hunter->id, hunter->boredom, hunter->fear, hunter->room->name, hunter->device_type);

    // Clears identified matching evidence in room
    room_evidence_clear(hunter->room, hunter->device_type);

    // Releases room evidence lock (here when matching evidence identified)
    sem_post(&(hunter->room->evidence_lock));

    // Waits for case file mutex
    sem_wait(&(hunter->case_file->mutex));

    // Adds evidence to shared case file
    casefile_evidence_add(hunter->case_file, hunter->device_type);

    // Releases case file mutex
    sem_post(&(hunter->case_file->mutex));

    // Checks to ensure hunter is not already in exit room
    if (!hunter_exit_check(hunter->room)) {

        hunter_return_exit(hunter, true);           // marks that hunter is required to return to exit
    }
}

/*
    Purpose:
        Compares hunter's device with the room's evidence, determines if there is any matching evidence.
    Parameters:
        - hunter (in): hunter structure
    Returns:
        True if device matches a piece of room evidence, false otherwise.
*/
bool hunter_check_evidence(Hunter *hunter) {

    bool evidence_match = evidence_byte_contains_type(hunter->room->evidence, hunter->device_type);         // compares room's evidence with hunter's device type

    return evidence_match;
}

/*
    Purpose:
        Tracks if hunter should return to exit (either because hunter identified evidence or by random chance).
    Parameters:
        - hunter (in/out): hunter structure
        - need_return (in): boolean indicating if hunter is required to return to exit room
*/
void hunter_return_exit(Hunter *hunter, bool need_return) {

    // Checks if hunter is required to return to evidence
    if (need_return) {

        hunter->return_to_van = true;

        // Logs hunter's start of returning to exit room
        log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->room->name, hunter->device_type, hunter->return_to_van);
    }
    // Gives hunter 19% chance of turning to exit room
    else {

        int rand_int = rand_int_threadsafe(0, 10);           // generates random integer between 0-9

        if (rand_int == 0) {

            hunter->return_to_van = true;

            // Logs hunter's start of returning to exit room
            log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->room->name, hunter->device_type, hunter->return_to_van);
        }
    }
}

/*
    Purpose:
        Attempts to execute action of hunter moving to next room.
    Parameters:
        - hunter (in/out): hunter structure
    Returns:
        C_OK if successful, C_ROOM_FULL if next room at full capacity, C_ERR otherwise.
*/
int hunter_move(Hunter *hunter) {

    Room *current_room = hunter->room;      // stores pointer to hunter's current room for logs
    Room *next_room;                        // stores pointer to next room hunter attempts to move to

    // Hunter is returning to van/exit room
    if (hunter->return_to_van) {

        next_room = roomstack_next_peek(&(hunter->rooms_path));     // gets next room below top of room path stack
    }
    // Hunter is exploring the house
    else {
        next_room = room_choose_rand_connection(hunter->room);      // randomly choose one of the current room's connected rooms
    }

    // Waits for room hunter occupancy locks (locks in order of memory addresses)
    if (current_room < next_room) {

        sem_wait(&(current_room->hunter_occupancy_lock));
        sem_wait(&(next_room->hunter_occupancy_lock));
    }
    else {
        sem_wait(&(next_room->hunter_occupancy_lock));
        sem_wait(&(current_room->hunter_occupancy_lock));
    }

    // Checks if next room is at full capacity
    if (next_room->hunter_arr.hunter_count >= MAX_ROOM_OCCUPANCY) {

        // TESTING
        // printf("MOVEMENT FAILED: Next room is full, hunter must remain in current room...\n");

        // Checks if hunter has just been initialized and was unable to be added to van hunter array and is still in van
        if (!(hunter->init_added_to_van) && hunter->init_first_room) {

            // Checks if van has capacity to add hunter to its fixed hunter array
            // Necessary to catch edge case where ghosts initializes in van and continues to stay there, needs to be able to detect hunters present in room
            if (hunter->room->hunter_arr.hunter_count < MAX_ROOM_OCCUPANCY) {

                hunter->init_added_to_van = true;


                fixed_hunterarr_add(&(hunter->room->hunter_arr), hunter);
            }
        }

        // Releases room hunter occupancy locks
        sem_post(&(current_room->hunter_occupancy_lock));
        sem_post(&(next_room->hunter_occupancy_lock));

        return C_ROOM_FULL;         //  movement fails, ends movement by returning so hunter remains in current room
    }

    // Removes hunter from current room and adds hunter to next room
    if (!room_remove_hunter(hunter->room, hunter)) {     
        printf("\nERROR: Hunter cannot be removed from current room...\n");
        return C_ERR;
    }    
    if (!room_add_hunter(next_room, hunter))  {
        printf("\nERROR: Hunter cannot be added to next room...\n");
        return C_ERR;
    }

    // Logs hunter's movement
    log_move(hunter->id, hunter->boredom, hunter->fear, current_room->name, next_room->name, hunter->device_type);

    // Releases room hunter occupancy locks
    sem_post(&(current_room->hunter_occupancy_lock));
    sem_post(&(next_room->hunter_occupancy_lock));

    // Checks if hunter was previously still in exit room after initialization
    if (hunter->init_first_room) {
        hunter->init_first_room = false;        // marks that hunter has left van after initializing
    }

    return C_OK;
}

// DYNAMIC HUNTER ARRAY FUNCTIONS

/*
    Purpose:
        Dynamically allocates memory for an dynamic array of hunter pointers.
        Initalizes fields of dynamic array structure storing hunters in the house.
    Parameters:
        - hunter_arr (out): dynamic array of hunters
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int dynamic_hunterarr_init(DynamicHunterArray *hunter_arr) {

    if (hunter_arr == NULL) {
        printf("\nERROR: Hunter dynamic array structure pointer is NULL, cannot initialize dynamic array structure...\n");
        return C_ERR;
    }

    hunter_arr->capacity = 7;           // provides initial capacity for storing 7 hunters (1 hunter per different device);
    hunter_arr->hunter_count = 0;       // initializes hunter count to 0

    hunter_arr->hunters = (Hunter**)malloc((hunter_arr->capacity)*sizeof(Hunter*));     // dynamically allocates memory for array of hunter pointers

    if (hunter_arr->hunters == NULL) {
        printf("\nERROR: Memory allocation error... \n");
        return C_ERR;
    }

    return C_OK;
}

/*
    Purpose: 
        Adds pointer to hunter allocated in heap to dynamic hunter array.
    Parameters:
        - hunter_arr (in/out): dynamic hunter array structure
        - hunter (in/out): hunter structure 
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int dynamic_hunterarr_add(DynamicHunterArray *hunter_arr, Hunter *hunter) {

    if ((hunter_arr == NULL) || (hunter == NULL)) {
        printf("\nERROR: Dynamic hunter array or hunter pointers are NULL, cannot add hunter to array...\n");
    }

    int success = 1;

    // Checks if dynamic hunter array has reached capacity
    if (hunter_arr->capacity == hunter_arr->hunter_count) {
        success = dynamic_hunterarr_grow(hunter_arr);               // grows capacity of dynamic hunter array
    }

    if (!success) {
        return C_ERR;
    }

    hunter_arr->hunters[hunter_arr->hunter_count] = hunter;         // double check if this is correct
    (hunter_arr->hunter_count)++;                                   // increments hunter count

    return C_OK;
}

/*
    Purpose:
        Grows capacity of dynamic array of hunters.
    Parameters:
        - hunter_arr (in/out): dynamic hunter array structure
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int dynamic_hunterarr_grow(DynamicHunterArray *hunter_arr) {

    int new_capacity = hunter_arr->capacity * 2;        // calculates double the current capacity of the dynamic hunter array

    hunter_arr->hunters = (Hunter**)realloc(hunter_arr->hunters, (new_capacity)*sizeof(Hunter*));       // dynamically reallocates memory for array of hunter pointers

    if (hunter_arr->hunters == NULL) {
        printf("\nERROR: Memory allocation error... \n");
        return C_ERR;
    }

    hunter_arr->capacity = new_capacity;                // updates capacity of dynamic hunter array
    return C_OK;
}

/*
    Purpose:
        Frees memory allocated for each hunter structure and the dynamic hunter array itself.
    Parameters:
        - hunter_arr (in/out): dynamic hunter array structure
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int dynamic_hunterarr_cleanup(DynamicHunterArray *hunter_arr) {

    // Checks if dynamic hunter array double pointer is NULL
    if (hunter_arr->hunters == NULL) {
        printf("\nERROR: Dynamic hunter array double pointer is NULL, cannot free allocated memory.\n");
        return C_ERR;
    }

    // Loops to free memory allocated for each hunter in dynamic array
    for (int i = 0; i < hunter_arr->hunter_count; i++) {
        
        hunter_cleanup(hunter_arr->hunters + i);
        hunter_arr->hunters[i] = NULL;                 
    }

    free(hunter_arr->hunters);                          // free dynamic hunter array
    hunter_arr->hunters = NULL;

    return C_OK;
}

// FIXED HUNTER ARRAY FUNCTIONS

/*
    Purpose:
        Initializes fixed hunter array stored in each room.
    Parameters:
        - hunter_arr (out): fixed hunter array structure
    Returns:
        C_OK if successfull, C_ERR otherwise.
*/
int fixed_hunterarr_init(FixedHunterArray *hunter_arr) {

    if (hunter_arr == NULL) {
        printf("\nERROR: Fixed hunter array pointer is NULL, cannot initialize fixed array structure...\n");
        return C_ERR;
    }

    hunter_arr->hunter_count = 0;
    return C_OK;
}

// Reused and modified from A4
/*
    Purpose:
        Adds hunter to provided fixed hunter array.
    Parameters:
        - hunter_arr (in/out): fixed hunter array structure
        - hunter (in): hunter structure
    Returns:
        C_OK if success, C_ERR otherwise.
*/
int fixed_hunterarr_add(FixedHunterArray *hunter_arr, Hunter *hunter) {

    if ((hunter_arr == NULL) || (hunter == NULL)) {
        printf("\nERROR: Room array structure or room pointer is NULL, cannot add hunter to room...\n");
        return C_ERR;
    }

    hunter_arr->hunters[hunter_arr->hunter_count] = hunter;         // appends hunter to provided room's fixed hunter array
    (hunter_arr->hunter_count)++;                                   // increments room hunter counter

    return C_OK;
}

/*
    Purpose:
        Removes hunter from provided fixed hunter array.
    Parameters:
        - hunter_arr (in/out): fixed hunter array structure
        - hunter (in): hunter structure
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int fixed_hunterarr_remove(FixedHunterArray *hunter_arr, const Hunter *hunter) {

    int hunter_index = fixed_hunterarr_get_hunter_pos(hunter_arr, hunter);

    if (hunter_index < 0) {
        printf("\nERROR: Hunter not found in fixed hunter array, cannot remove hunter from room...\n");
        return C_ERR;
    }

    // Shifts hunter pointers in fixed hunter array back to remove specified hunter from array
    for (int i = hunter_index; i < hunter_arr->hunter_count - 1; i++) {
        hunter_arr->hunters[i] = hunter_arr->hunters[i + 1];
    } 

    (hunter_arr->hunter_count)--;       // decrements room hunter counts

    return C_OK;
}   

/*
    Purpose: 
        Gets position specified hunter pointer is stored on fixed hunter array.
    Parameters:
        - hunter_arr (in): fixed hunter array structure
        - hunter (in): hunter structure
    Returns:
        Index of hunter pointer on array if found, C_NOT_FOUND otherwise.
*/
int fixed_hunterarr_get_hunter_pos(const FixedHunterArray *hunter_arr, const Hunter *hunter) {

    if ((hunter_arr == NULL) || (hunter == NULL)) {
        printf("\nERROR: Room array structure or room pointer is NULL, cannot identify hunter pointer position...\n");
        return C_NOT_FOUND;
    }

    // Loops through room's fixed hunter array to get position of hunter pointer
    for (int index = 0; index < hunter_arr->hunter_count; index++) {

        // Compares hunters to determine if matching
        if (hunter_compare(hunter, hunter_arr->hunters[index])) {

            return index;           // if hunter identified 
        }
    }

    return C_NOT_FOUND;
}

/*
    Purpose:
        Compares hunters to identify hunter pointer position on fixed hunter array.
        Identifies hunters as matching if their name and ID are the same.
    Parameters:
        - a (in): hunter structure a
        - b (in): hunter structure b
    Returns:
        True if hunters match, false otherwise.
*/
bool hunter_compare(const Hunter *a, const Hunter *b) {
    
    // Compares hunters by their names
    if (strcmp(a->name, b->name) != 0) {
        return false;
    }
    // Compares hunters by their ID value
    else if (a->id != b->id) {
        return false;
    }

    return true;
}

// RESULTS SCREEN PRINTING FUNCTIONS

/*
    Purpose:
        Prints a hunter's simulation results.
    Parameters:
        - hunter (in): hunter structure
*/
void hunter_result_print(const Hunter *hunter) {

    if (hunter->exited_reason == LR_EVIDENCE) {
        printf("[✔] ");
    }
    else {
        printf("[✘] ");
    }

    printf("Hunter %-15s (ID: %d) exited because of [%s] (bored=%d fear=%d)\n",
        hunter->name,
        hunter->id,
        exit_reason_to_string(hunter->exited_reason),
        hunter->boredom,
        hunter->fear);
}

/*
    Purpose:
        Prints all hunters' simulation results.
    Parameters:
        - hunter_arr (in): dynamic hunter array structure
*/
void hunters_all_result_print(const DynamicHunterArray *hunter_arr) {

    for (int i = 0; i < hunter_arr->hunter_count; i++) {
        hunter_result_print(hunter_arr->hunters[i]);
    }
}

/*
    Purpose:
        Counts how many hunters have identified the ghost before exiting the simulation.
    Parameters:
        - hunter_arr (in): dynamic hunter array structure
    Returns:
        Number of hunters that exited after identifying the ghost.
*/
int hunters_win_count(const DynamicHunterArray *hunter_arr) {

    int win_count = 0;

    for (int i = 0; i < hunter_arr->hunter_count; i++) {
        
        if(hunter_arr->hunters[i]->exited_reason == LR_EVIDENCE) {
            win_count++;
        }
    }

    return win_count;
}

// TESTING FUNCTIONS

void hunter_print(const Hunter *hunter) {

    // If associated room pointer is NULL
    if (hunter->room == NULL) {
        printf("\nNAME: %-20s | ID: %-10d | Device Type: %-10s | Current Room: %-15s\n", hunter->name, hunter->id, evidence_to_string(hunter->device_type), "Unknown");
    }
    // If associated room pointer has been properly initialized
    else {
        printf("\nNAME: %-20s | ID: %-10d | Device Type: %-10s | Current Room: %-15s\n", hunter->name, hunter->id, evidence_to_string(hunter->device_type), hunter->room->name);
    }
    
    printf("Hunter device type in byte form: ");
    print_bits((unsigned char)(hunter->device_type));
}