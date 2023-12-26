#ifndef GAMEPAD_UTIL_H
#define GAMEPAD_UTIL_H

#define NUM_BUTTONS 15
#define MAX_KEY_NAME_LENGTH 30

typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_A,
    BUTTON_B,
    BUTTON_X,
    BUTTON_Y,
    BUTTON_L1,
    BUTTON_L2,
    BUTTON_R1,
    BUTTON_R2,
    BUTTON_START,
    BUTTON_SELECT,
    BUTTON_MENU
} ButtonIndex;


typedef struct {
    const char *key;
    ButtonIndex index;
} gamepad_button_enum_map;

typedef struct {
    int state[NUM_BUTTONS];
} gamepad_state;

typedef struct {
    char* config[NUM_BUTTONS]; 
} gamepad_vis_img_path;

typedef struct {
    char* config[NUM_BUTTONS]; 
    int is_allocated[NUM_BUTTONS];
} gamepad_config;

extern const char* gamepad_body_vis_img_path;
extern const gamepad_vis_img_path vis_img_path; 
extern const gamepad_button_enum_map button_mapping[NUM_BUTTONS];

gamepad_state init_gamepad_state();
void load_gamepad_config(const char* filepath, gamepad_config* config);

#endif