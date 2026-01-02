#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_INPUT_STRING 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

#define C_NOT_FOUND -3
#define C_ROOM_FULL -2
#define C_ERR 0
#define C_OK 1
#define C_DONE 2

typedef unsigned char EvidenceByte; 	// just giving a helpful name to unsigned char for evidence bitmasks
typedef struct CaseFile CaseFile;

typedef struct House House;
typedef struct Ghost Ghost;
typedef struct Hunter Hunter;

typedef struct FixedHunterArray FixedHunterArray;
typedef struct DynamicHunterArray DynamicHunterArray;

typedef struct Room Room;
typedef struct RoomNode RoomNode;
typedef struct RoomStack RoomStack;

enum LogReason {
    LR_EVIDENCE = 0,
    LR_BORED = 1,
    LR_AFRAID = 2,
    LR_NOT_YET_EXIT = -1,       // acts as placeholder, never want it to actually appear on logs
};

enum EvidenceType {
    EV_EMF          = 1 << 0,
    EV_ORBS         = 1 << 1,
    EV_RADIO        = 1 << 2,
    EV_TEMPERATURE  = 1 << 3,
    EV_FINGERPRINTS = 1 << 4,
    EV_WRITING      = 1 << 5,
    EV_INFRARED     = 1 << 6,
};

enum GhostType {
    GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
    GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
    GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
    GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
    GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
    GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
    GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
    GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
    GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
    GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
    GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
    GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
    GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
    GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
    GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
    GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
    GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
    GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
    GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
    GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
    GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
    GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
    GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
    GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

// Should be allocated to heap
struct RoomNode {
    Room *room;      // points to room allocated on House room array
    RoomNode *next;
};

// Should be allocated to Hunter structure, singly-linked list should be used like a stack
struct RoomStack {
    RoomNode *head;  // points to head of singly-linked list of room nodes allocate on heap
};

// Dynamic array of hunters should be allocated to heap, with pointer to first hunter stored in House structure
struct DynamicHunterArray {
	int capacity;
	int hunter_count;
	Hunter **hunters;
};

// Should be allocated to each room structure
struct FixedHunterArray {
    Hunter *hunters[MAX_ROOM_OCCUPANCY];         
    int hunter_count;
};

// Should be allocated to House structure
struct CaseFile {
    EvidenceByte collected;     // union of all of the evidence bits collected between all hunters
    bool         solved;        // true when >=3 unique bits set
    sem_t        mutex;         // used for synchronizing both fields when multithreading
};

// Should be allocated to the House structure
struct Ghost {
	int id;
	enum GhostType type;
	Room *room;
	int boredom;
	bool running;       
	bool exited;   
    pthread_t thread;         
};

// Should be allocated to a dynamically allocated array, with pointer to first hunter stored on House structure
struct Hunter {
    char name[MAX_HUNTER_NAME];
    int id;
    Room *room;
    CaseFile *case_file;
    enum EvidenceType device_type;
    RoomStack rooms_path;              
    int boredom;
    int fear;
    enum LogReason exited_reason;
    bool init_first_room;
    bool init_added_to_van;
    bool return_to_van;
    bool running;
    bool exited;
    pthread_t thread;        
};

// Should all be allocated to House structure
struct Room {
    char name[MAX_ROOM_NAME];
    Room* rooms_connected[MAX_CONNECTIONS];
    int connect_count;  
    Ghost *ghost;
    FixedHunterArray hunter_arr;                  
    EvidenceByte evidence;
    sem_t evidence_lock;
    sem_t hunter_occupancy_lock; 
    sem_t ghost_presence_lock;                    
    bool is_exit;
};

// Can be either stack or heap allocated
struct House {
    Room *starting_room; 	// needed by house_populate_rooms, but can be adjusted to suit your needs.
    Ghost ghost;
    DynamicHunterArray hunter_arr;
    CaseFile case_file;
    bool entities_running;
    int room_count;  
    Room rooms[MAX_ROOMS];           
};

// House Functions
int house_create_stack(House *house);
void house_cleanup_stack(House *house);
int house_load_data(House *house);
int house_add_hunter(House *house, Hunter *hunter);
void house_check_entities_running(House *house);               // for single threading, do not think I will need for multi-threading

// Room Functions
int room_init(Room* room, const char* name, bool is_exit);
int room_connect(Room* a, Room* b);                           // bidirectional connection
Room* room_choose_rand_start(House *house);
Room* room_choose_rand_connection(Room *room);

// Room, Ghost, & Hunter Interaction Functions
void room_add_ghost(Room *room, Ghost *ghost);
void room_remove_ghost(Room *room, Ghost *ghost);
int room_add_hunter(Room *room, Hunter *hunter);
int room_remove_hunter(Room *room, Hunter *hunter);

// Room Evidence Functions
void room_evidence_add(Room *room, enum EvidenceType evidence);                
void room_evidence_clear(Room *room, enum EvidenceType evidence);

// RoomStack Functions
int roomstack_init(RoomStack *room_stack);
int roomstack_push(RoomStack *room_stack, Room *room);
int roomstack_pop(RoomStack *room_stack);
Room* roomstack_next_peek(const RoomStack *room_stack);                     
int roomstack_cleanup(RoomStack *room_stack, const bool exiting);

// Ghost Initialization Functions
int ghost_init(Ghost *ghost);
enum GhostType ghost_choose_rand_ghosttype();

// Ghost Thread Function
void *ghost_thread(void *arg);

// Ghost Take Turn Function
void ghost_take_turn(Ghost *ghost);

// Ghost Stats Functions
bool ghost_stats_update(Ghost *ghost);
bool ghost_check_hunters(const Room *room);
void ghost_boredom_inc(Ghost *ghost);
void ghost_boredom_reset(Ghost *ghost);
bool ghost_condition_check(Ghost *ghost);
void ghost_exit(Ghost *ghost);

// Ghost Behaviour Functions
int ghost_take_action(Ghost *ghost, bool can_move);
void ghost_idle(Ghost *ghost);
void ghost_haunt(Ghost *ghost);
void ghost_move(Ghost *ghost);

// Hunter Creation & Cleanup Functions
int hunter_user_create(House *house);
void get_str(char *output_str);
void get_int(int *output_int);
int hunter_init(Hunter* *hunter, const char* name, const int id, const bool chose_device, const int device_index);
enum EvidenceType hunter_choose_device(const bool chose_device, const int device_index);
int hunter_cleanup(Hunter* *hunter);

// Hunter Thread Function
void *hunter_thread(void *arg);

// Hunter Take Turn Function
void hunter_take_turn(Hunter *hunter);

// Hunter Stats Functions
void hunter_stats_update(Hunter *hunter);
bool hunter_check_ghost(const Room *room);
void hunter_boredom_inc(Hunter *hunter);
void hunter_boredom_reset(Hunter *hunter);
void hunter_fear_inc(Hunter *hunter);
bool hunter_condition_check(Hunter *hunter);
void hunter_exit(Hunter *hunter, enum LogReason exit_reason);

// Hunter Behaviour Functions
bool hunter_exit_check(const Room *room);
bool hunter_manage_exit_room(Hunter *hunter);
void hunter_swap_device(Hunter *hunter);
void hunter_gather_evidence(Hunter *hunter);
bool hunter_check_evidence(Hunter *hunter);
void hunter_return_exit(Hunter *hunter, bool need_return);
int hunter_move(Hunter *hunter);

// Dynamic Hunter Array Functions
int dynamic_hunterarr_init(DynamicHunterArray *hunter_arr);
int dynamic_hunterarr_add(DynamicHunterArray *hunter_arr, Hunter *hunter);
int dynamic_hunterarr_grow(DynamicHunterArray *hunter_arr);
int dynamic_hunterarr_cleanup(DynamicHunterArray *hunter_arr); 

// Fixed Hunter Array Functions
int fixed_hunterarr_init(FixedHunterArray *hunter_arr);
int fixed_hunterarr_add(FixedHunterArray *hunter_arr, Hunter *hunter);
int fixed_hunterarr_remove(FixedHunterArray *hunter_arr, const Hunter *hunter);
int fixed_hunterarr_get_hunter_pos(const FixedHunterArray *hunter_arr, const Hunter *hunter);
bool hunter_compare(const Hunter *a, const Hunter *b);

// Evidence Functions
int casefile_init(CaseFile *case_file);
bool casefile_check_victory(const CaseFile *case_file);
void casefile_solved(CaseFile *case_file);
void casefile_evidence_add(CaseFile *case_file, enum EvidenceType evidence);
void ghost_to_evidence_types(const Ghost *ghost, enum EvidenceType ghost_evidence_types[]);
bool evidence_byte_contains_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence);
EvidenceByte evidence_byte_set_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence);
EvidenceByte evidence_byte_clear_type(const EvidenceByte evidence_byte, const enum EvidenceType evidence);

// Bitwise Operators Helper Functions
unsigned char get_bit(unsigned char c, int n);
unsigned char set_bit(unsigned char c, int n);
unsigned char clear_bit(unsigned char c, int n);

// Results Screen Functions
void casefile_results_print(CaseFile *case_file);
void hunter_result_print(const Hunter *hunter);
void hunters_all_result_print(const DynamicHunterArray *hunter_arr);
int hunters_win_count(const DynamicHunterArray *hunter_arr);

// Testing Functions
void house_print_rooms(const House *house);
void house_print_ghost(const House *house);
void house_print_hunters(const House *house);
void room_print(const Room *room);
void ghost_print(const Ghost *ghost);
void hunter_print(const Hunter *hunter);
void roomstack_print(RoomStack *room_stack);
void print_bits(unsigned char c);

#endif // DEFS_H
