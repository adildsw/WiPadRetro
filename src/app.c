#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <pthread.h>

#include <config_util.h>
#include <ip_util.h>
#include <tcp_server.h>

const int WINDOW_WIDTH = 320;
const int WINDOW_HEIGHT = 240;

SDL_Surface *screen = NULL;
SDL_Surface *buffer = NULL;
SDL_bool redraw = SDL_TRUE;

TTF_Font *font;
TTF_Font *font_bold;

SDL_Color white = {255, 255, 255};

SDL_bool running = SDL_TRUE;

int ip_count;
char **ip_addresses;
int ip_index = 0;

char client_ip_address[16] = "";
SDL_bool is_connected = SDL_FALSE;

SDL_bool is_up = SDL_FALSE;

gamepad_state state;
gamepad_config config;

char* debug_text = "Debug text";
char* control_help = "L/R: Cycle Network | MENU+START: Exit App";

void init();
void render_text(SDL_Surface *screen, const char *text, int x, int y, TTF_Font *font, SDL_Color color);
void render_image(SDL_Surface* screen, const char* imagePath, int x, int y);
void render_title();
void render_topbar(const char* text);
void render_bottombar();
void render_input_visualization();
void handle_input(SDL_Event* event);
void stream_input_to_client();
void cycle_network();
void render();
void tcp_callback(const char* data);
void cleanup();


// ---------------- Initialization ----------------
void init()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) == -1)
    {
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    screen = SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, 32, SDL_HWSURFACE | SDL_DOUBLEBUF);
    buffer = SDL_CreateRGBSurface(SDL_HWSURFACE, WINDOW_WIDTH, WINDOW_HEIGHT, 32, 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    if (!screen || !buffer)
    {
        fprintf(stderr, "Could not set video mode: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    if (TTF_Init() == -1)
    {
        fprintf(stderr, "TTF_Init: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    font = TTF_OpenFont("./assets/fonts/rainyhearts.ttf", 16);
    font_bold = TTF_OpenFont("./assets/fonts/Pixellari.ttf", 16);
    if (!font || !font_bold)
    {
        fprintf(stderr, "TTF_OpenFont: %s\n", TTF_GetError());
        SDL_Quit();
        return;
    }

    ip_addresses = get_ip_addresses(&ip_count);
    if (!ip_addresses)
    {
        fprintf(stderr, "Failed to get IP addresses\n");
        SDL_Quit();
        return;
    }
    cycle_network();

    state = init_gamepad_state();
    load_gamepad_config(&config);

    start_tcp_server(tcp_callback);
}


// ---------------- Render Functions ----------------
void render_text(SDL_Surface *screen, const char *text, int x, int y, TTF_Font *font, SDL_Color color)
{
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, text, color);
    if (!textSurface)
    {
        fprintf(stderr, "Failed to render text: %s\n", TTF_GetError());
        return;
    }
    SDL_Rect textRect = {x, y, textSurface->w, textSurface->h};

    SDL_BlitSurface(textSurface, NULL, screen, &textRect);
    SDL_FreeSurface(textSurface);
}

void render_image(SDL_Surface* screen, const char* imagePath, int x, int y) {
    SDL_Surface* loadedImage = IMG_Load(imagePath);
    if (!loadedImage) {
        fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", imagePath, IMG_GetError());
        return; 
    }
    SDL_Rect destRect;
    destRect.x = x;
    destRect.y = y;
    destRect.w = loadedImage->w;
    destRect.h = loadedImage->h; 

    SDL_BlitSurface(loadedImage, NULL, screen, &destRect);
    SDL_FreeSurface(loadedImage);
}

void render_title() {
    SDL_Rect topBarRect;
    topBarRect.x = 0;
    topBarRect.y = 0;
    topBarRect.w = 125;
    topBarRect.h = 20;

    if (is_connected)
        SDL_FillRect(buffer, &topBarRect, SDL_MapRGB(buffer->format, 34, 139, 34));
    else 
        SDL_FillRect(buffer, &topBarRect, SDL_MapRGB(buffer->format, 255, 102, 102));
    render_text(buffer, "WiPadRetro v0.1", 5, 3, font_bold, white);
}

void render_topbar(const char* text) {
    SDL_Rect con_bar_rect;
    con_bar_rect.x = 125;
    con_bar_rect.y = 0;
    con_bar_rect.w = WINDOW_WIDTH - 125;
    con_bar_rect.h = 20;

    SDL_FillRect(buffer, &con_bar_rect, SDL_MapRGB(buffer->format, 230, 230, 230));
    SDL_Color c = {0, 0, 0};

    if (is_connected)
    {
        render_text(buffer, "Connected: ", 130, 3, font, c);
        render_text(buffer, client_ip_address, 200, 3, font, c);
    }
    else
    {
        render_text(buffer, "Device: ", 130, 3, font, c);
        render_text(buffer, ip_addresses[ip_index], 175, 3, font, c);
    }
}

void render_bottombar() {
    SDL_Rect help_bar_rect;
    help_bar_rect.x = 0;
    help_bar_rect.y = WINDOW_HEIGHT - 20;
    help_bar_rect.w = WINDOW_WIDTH;
    help_bar_rect.h = 20;

    SDL_FillRect(buffer, &help_bar_rect, SDL_MapRGB(buffer->format, 48, 48, 48));

    if (is_connected) {
        control_help = "MENU+START: Disconnect";
    }
    else {
        if (ip_count <= 2) {
            control_help = "MENU+START: Exit App";
        } else {
            control_help = "SELECT: Cycle Network | MENU+START: Exit App";
        }
    }

    render_text(buffer, control_help, 5, WINDOW_HEIGHT - 17, font, white);
}

void render_input_visualization() {
    render_image(buffer, gamepad_body_vis_img_path, 0, 0);
    if (!is_connected) return;

    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (state.state[i]) {
            render_image(buffer, vis_img_path.config[i], 0, 0);
        }
    }
}

void render() {
    SDL_FillRect(buffer, NULL, SDL_MapRGB(buffer->format, 0, 0, 0));
    render_title();
    render_topbar("Network: 127.0.0.1");
    render_bottombar();
    render_input_visualization();

    SDL_BlitSurface(buffer, NULL, screen, NULL);
    SDL_Flip(screen);
    redraw = SDL_FALSE;
}


// ---------------- Input Handling ----------------
void handle_input(SDL_Event* event) {
    if (event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        int stateChange = (event->type == SDL_KEYDOWN) ? 1 : 0;

        const char* keyName = SDL_GetKeyName(event->key.keysym.sym);
        
        for (int i = 0; i < NUM_BUTTONS; i++) {
            if (strcmp(keyName, config.config[i]) == 0) {
                state.state[i] = stateChange;
                redraw = SDL_TRUE;
                break;
            }
        }

        // Send Input To Client
        if (is_connected) {
            stream_input_to_client();
        }

        // App Controls
        if (state.state[BUTTON_MENU] && state.state[BUTTON_START]) {
            if (is_connected) {
                force_disconnect_client();
                is_connected = SDL_FALSE;
                redraw = SDL_TRUE;
            } else {
                running = SDL_FALSE;
                redraw = SDL_TRUE;
            }
        }
        if (state.state[BUTTON_SELECT]) {
            cycle_network();
            redraw = SDL_TRUE;
        }
    }
}

void stream_input_to_client() {
    // char data[200];
    // strcpy(data, "{\n");
    // for (int i = 0; i < NUM_BUTTONS - 1; i++) {
    //     char button_state[15];
    //     sprintf(button_state, "\t\"%s\": %d,\n", button_mapping[i].key, state.state[button_mapping[i].index]);
    //     strcat(data, button_state);
    // }
    // data[strlen(data) - 2] = '\0';
    // strcat(data, "}\n");
    // udp_send_to_client(data);

    unsigned short packed = 0;
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (state.state[button_mapping[i].index]) {
            packed |= (1 << i);
        }
    }
    udp_send_to_client(packed);
}

void cycle_network() {
    if (ip_count <= 1) return;
    if (ip_count == 2) {
        for (int i = 0; i < ip_count; i++) {
            if (strcmp(ip_addresses[i], "127.0.0.1") == 0) {
                ip_index = (i + 1) % ip_count;
                break;
            }
        }
        return;
    }

    ip_index = (ip_index + 1) % ip_count;
    if (strcmp(ip_addresses[ip_index], "127.0.0.1") == 0)
    ip_index = (ip_index + 1) % ip_count;
}

void tcp_callback(const char* data) {
    printf("Data received: %s\n", data);

    if (strncmp(data, "CLIENT:", 7) == 0) {
        send_to_client("WIPADRETRO_CONNECTED");
        strcpy(client_ip_address, data + 7);
        is_connected = SDL_TRUE;
        redraw = SDL_TRUE;
    }

    if (strcmp(data, "DISCONNECTED") == 0) {
        is_connected = SDL_FALSE;
        redraw = SDL_TRUE;
        start_tcp_server(tcp_callback);
    }
}


// ---------------- Main and Cleanup ----------------
int main(int argc, char *argv[])
{
    // Initializing app
    init();

    // Main SDL loop
    SDL_Event event;
    while (running)
    {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
                break;
            }

            // Handling input
            handle_input(&event);
        }

        // Rendering
        if (redraw) render();
        if (is_connected) {
            stream_input_to_client();
        }

        // Delaying to 60 FPS
        SDL_Delay(1000 / 60);
    }
    cleanup();
    return 0;
}

void cleanup()
{
    // Stopping TCP server
    stop_tcp_server();

    // Cleaning up IP string allocations
    if (ip_addresses)
    {
        for (int i = 0; i < ip_count; i++)
        {
            free(ip_addresses[i]);
        }
        free(ip_addresses);
    }

    // Cleaning up gamepad configuration allocations
    for (int i = 0; i < NUM_BUTTONS; i++) {
        if (config.is_allocated[i]) {
            free(config.config[i]);
            config.config[i] = NULL;
        }
    }

    // Cleaning up screen buffers
    if (screen) SDL_FreeSurface(screen);
    if (buffer) SDL_FreeSurface(buffer);

    // Cleaning up SDL
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_Quit();
}


