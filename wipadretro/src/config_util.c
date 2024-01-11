#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <config_util.h>

const char* gamepad_body_vis_img_path = "./assets/screen/main.png";

const gamepad_vis_img_path vis_img_path = {
    .config[BUTTON_UP] = "./assets/screen/up.png",
    .config[BUTTON_DOWN] = "./assets/screen/down.png",
    .config[BUTTON_LEFT] = "./assets/screen/left.png",
    .config[BUTTON_RIGHT] = "./assets/screen/right.png",
    .config[BUTTON_A] = "./assets/screen/a.png",
    .config[BUTTON_B] = "./assets/screen/b.png",
    .config[BUTTON_X] = "./assets/screen/x.png",
    .config[BUTTON_Y] = "./assets/screen/y.png",
    .config[BUTTON_L1] = "./assets/screen/l1.png",
    .config[BUTTON_L2] = "./assets/screen/l2.png",
    .config[BUTTON_R1] = "./assets/screen/r1.png",
    .config[BUTTON_R2] = "./assets/screen/r2.png",
    .config[BUTTON_START] = "./assets/screen/start.png",
    .config[BUTTON_SELECT] = "./assets/screen/select.png",
    .config[BUTTON_MENU] = "./assets/screen/menu.png"
};

const gamepad_button_enum_map button_mapping[NUM_BUTTONS] = {
    {"up", BUTTON_UP},
    {"down", BUTTON_DOWN},
    {"left", BUTTON_LEFT},
    {"right", BUTTON_RIGHT},
    {"a", BUTTON_A},
    {"b", BUTTON_B},
    {"x", BUTTON_X},
    {"y", BUTTON_Y},
    {"l1", BUTTON_L1},
    {"l2", BUTTON_L2},
    {"r1", BUTTON_R1},
    {"r2", BUTTON_R2},
    {"start", BUTTON_START},
    {"select", BUTTON_SELECT},
    {"menu", BUTTON_MENU}
};

gamepad_state init_gamepad_state(void) {
    gamepad_state state;
    memset(&state, 0, sizeof(state));
    return state;
}

void load_gamepad_config(gamepad_config* config) {
    memset(config->is_allocated, 0, sizeof(config->is_allocated));
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (config->config[i] != NULL) {
            free(config->config[i]);
            config->config[i] = NULL;
        }
    }

    const char* filename = "config.ini";

    create_gamepad_config_file(filename);
    FILE *file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "Unable to open the config file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    char line[256];
    char key[50];
    char value[MAX_KEY_NAME_LENGTH];

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\n")] = 0;

        if (sscanf(line, "%49[^=]=%49[^\n]", key, value) == 2) {
            for (int i = 0; i < NUM_BUTTONS; i++) {
                if (strcmp(key, button_mapping[i].key) == 0) {
                    config->config[button_mapping[i].index] = malloc((strlen(value) + 1) * sizeof(char));
                    if (config->config[button_mapping[i].index] != NULL) {
                        strcpy(config->config[button_mapping[i].index], value);
                        config->is_allocated[button_mapping[i].index] = 1;
                    } else {
                        fprintf(stderr, "Failed to allocate memory for button %d\n", i);
                    }
                    break;
                }
            }
        }
    }
    fclose(file);
}

void create_gamepad_config_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
    } else {
        if (errno == ENOENT) {
            file = fopen(filename, "w");
            if (file == NULL) {
                perror("Error creating file");
                exit(EXIT_FAILURE);
            }
            
            fprintf(file, "up=up\ndown=down\nleft=left\nright=right\n");
            fprintf(file, "a=space\nb=left ctrl\nx=left shift\ny=left alt\n");
            fprintf(file, "l1=e\nl2=tab\nr1=t\nr2=backspace\n");
            fprintf(file, "start=return\nselect=right ctrl\nmenu=escape\n");

            fclose(file);
            printf("File '%s' created with default settings.\n", filename);
        } else {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }
    }
}