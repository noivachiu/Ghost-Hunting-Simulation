#include <stdio.h>
#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

/*
    Purpose: 
        Initializes fields of a case file structure.
    Parameters:
        - case_file (out): case file tracking hunters' gathered evidence
    Returns:
        C_ERR if error occurs, C_OK if successful.
*/
int casefile_init(CaseFile *case_file) {

    if (case_file == NULL) {
        printf("ERROR: Case file pointer is NULL, cannot initialize case file.\n");
        return C_ERR;
    }

    // Initializes semaphore/mutex
    if (sem_init(&(case_file->mutex), 0, 1)) {
        printf("\nERROR: On semaphore init...\n");
        exit(1);
    }

    // Initializes case file fields to simulation starting values
    case_file->collected = 0;
    case_file->solved = false;

    return C_OK;
}

/*
    Purpose:
        Adds hunter identified evidence to shared case file.
    Parameters:
        - case_file (in/out): case file structure
        - evidence (in): evidence to add to file
*/
void casefile_evidence_add(CaseFile *case_file, enum EvidenceType evidence) {

    case_file->collected = evidence_byte_set_type(case_file->collected, evidence);
}

/*
    Purpose:
        Checks shared case file to determine if hunter has identified a valid ghost.
    Parameters:
        - case_file (in): case file structure
    Returns:
        - True if valid ghost is identified, false otherwise.
*/
bool casefile_check_victory(const CaseFile *case_file) {

    if (case_file->solved) {
        return true;
    }

    // Checks if case file contains 3 unique pieces of evidence
    if (!evidence_has_three_unique(case_file->collected)) {
        return false;
    }

    // Checks if evidence matches a valid ghost type
    bool is_valid_ghost = evidence_is_valid_ghost(case_file->collected);

    return is_valid_ghost;
}

/*
    Purpose:
        Marks that the shared case file has been solved.
    Parameters:
        - case_file (out): case file structure
*/
void casefile_solved(CaseFile *case_file) {
    
    case_file->solved = true;
}

/*
    Purpose: 
        Populates provided array with all 3 individual pieces of evidence that identify a ghost.
    Parameters:
        - ghost (in): ghost structure
        - ghost_evidence_types (out): array to be populated with the individual evidence types that make up provided ghost
    Returns:
        Array of the provided ghost's evidence types.
*/
void ghost_to_evidence_types(const Ghost *ghost, enum EvidenceType ghost_evidence_types[]) {

    // Gets list of all evidence types
    const enum EvidenceType* evidence_types = NULL;
    int evidence_count = get_all_evidence_types(&evidence_types);

    int identify_count = 0; // counter to track number of evidence that makes up ghost identified

    // Loops through evidence types list to find all types that identify ghost
    for (int i = 0; i < evidence_count; i++) {

        if (evidence_byte_contains_type((EvidenceByte)(ghost->type), evidence_types[i])) {

            ghost_evidence_types[identify_count] = evidence_types[i];       // stores type on provided array
            identify_count++;
        }

        if (identify_count == 3) {                                          // if have identified all evidence types
            break;
        }
    }
}

/*
    Purpose:
        Indicates if provided evidence type is contained in evidence byte (get bit).
    Parameters:
        - evidence_byte (in): evidence byte (room evidence or ghost type)
        - evidence (in): evidence type
    Returns:
        True if evidence byte contains evidence type, false otherwise.
*/
bool evidence_byte_contains_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence) {

    bool contains_type = (enum EvidenceType)(evidence_byte & evidence) == evidence;

    return contains_type;
}

/*
    Purpose:
        Combines provided evidence type with evidence byte (set bit).
    Parameters:
        - evidence_byte (in): evidence byte (room evidence or collected case file evidence)
        - evidence (in): evidence type
    Returns:
        Updated evidence byte after performing bitwise operations.
*/
EvidenceByte evidence_byte_set_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence) {
    
    return (EvidenceByte)(evidence_byte | evidence);        // returns evidence byte with evidence type set
}

/*
    Purpose:
        Clears provided evidence type in evidence byte (clear bit).
    Parameters:
        - evidence_byte (in): evidence byte (room evidence or collected case file evidence)
        - evidence (in): evidence type
    Returns:
        Updated evidence byte after performing bitwise operations.
*/
EvidenceByte evidence_byte_clear_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence) {
    
    return (EvidenceByte)(evidence_byte & (~evidence));
}

// BITWISE OPERATORS HELPER FUNCTIONS

// Reused from A3
/*
    Purpose: 
        Gets bit at the nth position of the byte
    Parameters:
        -  c (in): Byte to get bit from
        -  n (in): The position to get the bit at  
    Return: 
        The bit at the nth position.
*/
unsigned char get_bit(unsigned char c, int n) {

    unsigned char m = 1 << n;         // Creates bitmask

    return (c & m) >> n;              // Returns char with nth bit read and right shifted by n
}

// Reused from A3
/* 
    Purpose: 
        Sets bit at the nth position of the byte.
    Parameters:
        -  c (in): Byte to set the bit
        -  n (in): The position to set the bit at
    Returns: 
        The value of byte c with bit set at position n.
*/
unsigned char set_bit(unsigned char c, int n) {

    unsigned char m = 1 << n;         // Creates bitmask

    return c | m;                     // Applies bitmask and returns char c with nth bit set
}

// Reused from A3
/* 
    Purpose: 
        Clears bit at the nth position of the byte.
    Parameters:
        -  c (in): Byte to clear the bit
        -  n (in): The position to set the bit at
    Returns: 
        The value of byte c with bit clear at position n.
*/
unsigned char clear_bit(unsigned char c, int n) {

    unsigned char m = ~(1 << n);      // Creates bitmask

    return c & m;                     // Applies bitmask and returns char c with nth bit cleared
}

// RESULTS SCREEN PRINTING FUNCTIONS
void casefile_results_print(CaseFile *case_file) {

    // Gets list of all evidence types
    const enum EvidenceType* evidence_types = NULL;
    int evidence_count = get_all_evidence_types(&evidence_types);

    printf("\nShared Case File Checklist: \n");

    // Loops to read each bit (evidence type) of the case file's collected evidence byte
    for (int i = 0; i < evidence_count; i++) {

        unsigned char bit = get_bit((unsigned char)case_file->collected, i);       

        // Prints if evidence of that type was collected
        if (bit) {
            printf("    - [✔] ");
        }
        else {
            printf("    - [ ] ");
        }

        printf("%s \n", evidence_to_string(evidence_types[i]));        // prints evidence type
    }
}

// TESTING FUNCTIONS

void print_bits(unsigned char c) {

  // Loops to print bits of char c
  for (int i = (sizeof(c) * 8) - 1; i >= 0 ; i--) {

    unsigned char bit = get_bit(c, i);   // Calls get_bit() to get each individual bit

    printf("%u", bit);                  // Prints each bit
  }

  printf("\n");
}