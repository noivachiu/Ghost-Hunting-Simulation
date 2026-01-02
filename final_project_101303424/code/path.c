#include <stdio.h>
#include <stdlib.h>
#include "defs.h" 

// ROOM STACK FUNCTIONS

// Reused and modified from A4
/*
    Purpose:
        Initializes an empty singly linked list that stores each hunter room path stack.
    Parameters:
        - room_stack (out): hunter room path stack
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int roomstack_init(RoomStack *room_stack) {

    if (room_stack == NULL) {
        printf("ERROR: Room stack pointer is NULL, cannot initialize.\n");
        return C_ERR;
    }

    room_stack->head = NULL;      // initializes head pointer to NULL

    return C_OK;
}

// Reused and modified from A4
/*
    Purpose: 
        Adds provided room to top of provided hunter room path stack.
    Parameters:
        - room_stack (in/out): hunter room path stack
        - room (out): room structure, to push onto room stack
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int roomstack_push(RoomStack *room_stack, Room *room) {

    if ((room_stack == NULL) || (room == NULL)) {
        
        printf("\nERROR: Room stack or room pointers are NULL, cannot push room to stack...\n");
        return C_ERR;
    }
    
    RoomNode *new_node = (RoomNode*) malloc(sizeof(RoomNode));     // dynamically allocated a room node

    if (new_node == NULL) {
        printf("\nERROR: Memory allocation error... \n");
        return C_ERR;
    }

    // Initializes fields of new node
    new_node->room = room;

    // Pushes new room node onto room stack by setting it as new head
    new_node->next = room_stack->head;
    room_stack->head = new_node;
    
    return C_OK;
}

// Reused and modified from A4
/*
    Purpose: 
        Removes provided room at top of provided hunter room path stack.
    Parameters:
        - room_stack (in/out): hunter room path stack
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int roomstack_pop(RoomStack *room_stack) {

    if (room_stack == NULL) {
        printf("\nERROR: Room stack pointer is NULL, cannot pop room from stack...\n");
        return C_ERR;
    }
    else if (room_stack->head == NULL) {
        printf("\nERROR: Room stack is empty, cannot pop room from stack...\n");
        return C_ERR;
    }

    struct RoomNode *current_head = room_stack->head;        // stores pointer to current head of stack
    room_stack->head = room_stack->head->next;               // sets next room node as new head

    free(current_head);                                      // frees current head

    return C_OK;
}

/*
    Purpose: 
        Returns the next room below the top room of the room path stack.
        Indicates a hunter's next room to move to if retracing room path and returning to van.
    Parameters:
        - room_stack (in): hunter room path stack
    Returns:
        Next room hunter should move into, otherwise NULL if stack is NULL, empty or hunter is in van/exit room.
*/
Room* roomstack_next_peek(const RoomStack *room_stack) {

    if (room_stack == NULL) {
        printf("\nERROR: Room stack pointer is NULL, cannot peek at current room...\n");
        return NULL;
    }

    return room_stack->head->next->room;
}

// Reused and modified from A4
/*
    Purpose:
        Frees memory associated with all nodes in room path stack, except for initial exit room visit if hunter is still running.
    Parameters:
        - room_stack (in/out): hunter room path stack
        - exiting (in): indicates if hunter is exiting the simulation
    Returns:
        C_OK if successful, C_ERR otherwise.
*/
int roomstack_cleanup(RoomStack *room_stack, const bool exiting) {

    if (room_stack == NULL) {
        printf("\nERROR: Room stack pointer is NULL, cannot cleanup stack...\n");
        return C_ERR;
    }
    else if (room_stack->head == NULL) {
        printf("\nERROR: Room stack is empty, cannot cleanup stack...\n");
        return C_ERR;
    }

    // If hunter is exiting the simulation
    if (exiting) {
        while (room_stack->head != NULL) {
            roomstack_pop(room_stack);
        }
    }
    // If hunter is in van/exit room but is still running, keep last room (exit room)
    else {
        while (room_stack->head->next != NULL) {
            roomstack_pop(room_stack);
        }
    }

    return C_OK;
}

// TESTING FUNCTIONS

void roomstack_print(RoomStack *room_stack) {

    // Checks if list pointer is NULL
    if (room_stack == NULL) {
        printf("\nERROR: Room stack pointer is NULL, cannot print rooms from stack...\n");
        return;
    }
    // Checks if list is empty
    else if (room_stack->head == NULL) {
        printf("ERROR: Room stack head pointer is NULL, no rooms to print.\n");
        return;
    }

    RoomNode *current = room_stack->head;     // stores current node to iterate through linked list

    printf("\n");

    // Loops through linked list to print room names until current node is NULL
    while (current != NULL) {

        printf("%s -> ", current->room->name);
        current = current->next;
    }

    printf("null \n\n");
}