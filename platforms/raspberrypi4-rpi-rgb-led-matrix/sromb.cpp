#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <cmath>
#include <sys/time.h>
#include <SDL2/SDL.h>
#include <libconfig.h++>
#include <led-matrix.h>
#include <boost/filesystem.hpp> 

using namespace std;
using namespace libconfig;
using namespace rgb_matrix;
namespace fs = ::boost::filesystem;

#define SCREEN_FPS 59.7
const uint32_t SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // (uint32_t) round()

bool running = true;
bool paused = false;

static const char *output_file = "gearboy.cfg";

bool audioEnabled = true;

typedef uint8_t u8;

struct palette_color
{
    int red;
    int green;
    int blue;
    int alpha;
};

palette_color dmg_palette[4];
#define DEBUG_GEARBOY

#ifdef DEBUG_GEARBOY
    #ifdef __ANDROID__
        #include <android/log.h>
        #define printf(...) __android_log_print(ANDROID_LOG_DEBUG, "GEARBOY", __VA_ARGS__);
    #endif
#define Log(msg, ...) (Log_func(msg, ##__VA_ARGS__))
#else
#define Log(msg, ...)
#endif
#define GAMEBOY_WIDTH 160
#define GAMEBOY_HEIGHT 144

inline void Log_func(const char* const msg, ...)
{
    static int count = 1;
    char szBuf[512];

    va_list args;
    va_start(args, msg);
    vsprintf(szBuf, msg, args);
    va_end(args);

    printf("%d: %s\n", count, szBuf);

    count++;
}

struct GB_Color
{
#if defined(__LIBRETRO__)
    #if defined(IS_LITTLE_ENDIAN)
    u8 blue;
    u8 green;
    u8 red;
    u8 alpha;
    #elif defined(IS_BIG_ENDIAN)
    u8 alpha;
    u8 red;
    u8 green;
    u8 blue;
    #endif
#else
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
#endif
};



SDL_Joystick* game_pad = NULL;
SDL_Keycode kc_keypad_left, kc_keypad_right, kc_keypad_up, kc_keypad_down, kc_keypad_a, kc_keypad_b, kc_keypad_start, kc_keypad_select, kc_emulator_pause, kc_emulator_quit;
bool jg_x_axis_invert, jg_y_axis_invert;
int jg_a, jg_b, jg_start, jg_select, jg_x_axis, jg_y_axis;

uint32_t screen_width, screen_height;

bool noWindow = false;
SDL_Window* theWindow;
SDL_Renderer* theRenderer;
SDL_Texture* theScreen;

rgb_matrix::RGBMatrix::Options matrix_options;
uint32_t matrix_width;
uint32_t matrix_height;
RGBMatrix* matrix;
FrameCanvas* offscreen_canvas;
uint32_t timer_fps_cap_ticks_start = 0;

uint32_t totalPixels = GAMEBOY_WIDTH * GAMEBOY_HEIGHT;
uint32_t skipx = 5; // 160/(160-128)
uint32_t skipy = 8; // (144/(144-128))-1)


void update_matrix(void){

    for(uint32_t i=0; i < totalPixels; i++ ){
        uint32_t fbX = i % GAMEBOY_WIDTH;
        uint32_t fbY = i / GAMEBOY_WIDTH;


        if(fbX > 0 && fbX % skipx == 0 ){ // hard coded value for testing: 128 / 160 = 0.8 = 80% = Render 8 out of 10 pixels = render 4 out of 5, so skip every 5th!
            continue;
        }
        if(fbY > 0 && fbY % skipy == 0){ // hard coded value for testing: 128 / 144 = 0.88888888888 = 88% (skip 12 when 100 -> skip 1 when 8.33333333333)
            continue;
        }
        /*GB_Color pixelColor = theFrameBuffer[i];

        applySelectiveSkippedColorInterpolation(&pixelColor, i, fbX, fbY);

        uint32_t matrixX = fbX - fbX/skipx;
        uint32_t matrixY = fbY - fbY/skipy;
        if(matrixY >= matrix_height){
            break;
        }else if(matrixX < matrix_width){
            offscreen_canvas->SetPixel(matrixX, matrixY, pixelColor.red, pixelColor.green, pixelColor.blue);
        }*/
    }
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
}

void update(void)
{
    
    timer_fps_cap_ticks_start = SDL_GetTicks();
    SDL_Event keyevent;

    while (SDL_PollEvent(&keyevent))
    {
        switch(keyevent.type)
        {
            case SDL_QUIT:
            running = false;
            Log("EXITING!!!");
            break;

            /*case SDL_JOYBUTTONDOWN:
            {
                if (keyevent.jbutton.button == jg_b)

                else if (keyevent.jbutton.button == jg_a)

                else if (keyevent.jbutton.button == jg_select)

                else if (keyevent.jbutton.button == jg_start)
 
            }
            break;

            case SDL_JOYBUTTONUP:
            {
                if (keyevent.jbutton.button == jg_b)
;
                else if (keyevent.jbutton.button == jg_a)

                else if (keyevent.jbutton.button == jg_select)

                else if (keyevent.jbutton.button == jg_start)

            }
            break;

            case SDL_KEYDOWN:
            {
                if (keyevent.key.keysym.sym == kc_keypad_left)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_right)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_up)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_down)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_b)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_a)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_select)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_start)
                    

                if (keyevent.key.keysym.sym == kc_emulator_quit)
                    running = false;
                else if (keyevent.key.keysym.sym == kc_emulator_pause)
                {
                    
                }
            }
            break;

            case SDL_KEYUP:
            {
                if (keyevent.key.keysym.sym == kc_keypad_left)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_right)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_up)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_down)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_b)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_a)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_select)
                    
                else if (keyevent.key.keysym.sym == kc_keypad_start)
                    
            }
            break;*/
        }
    }

    if(!noWindow){
        SDL_RenderClear(theRenderer);

        // Render in SDL

        SDL_RenderCopy(theRenderer, theScreen, NULL, NULL);

        SDL_RenderPresent(theRenderer);
    }

    update_matrix();

    uint32_t frameticks = SDL_GetTicks() - timer_fps_cap_ticks_start;
    if (frameticks < SCREEN_TICKS_PER_FRAME) {
        SDL_Delay(SCREEN_TICKS_PER_FRAME - frameticks);
    }
    
}

void init_matrix(int argc, char** argv){
        // Initialize from flags.

        rgb_matrix::RuntimeOptions runtime_options;
        runtime_options.drop_privileges = -1;  // Need root
        if (!rgb_matrix::ParseOptionsFromFlags(&argc, &argv,
                                                &matrix_options, &runtime_options)) {

            Log("RGB_MATRIX Complains over options!");
            //usage(argv[0]);
            //return 1;
        }
        // Initialize matrix library.
    // Create matrix and apply GridTransformer.
    matrix = CreateMatrixFromOptions(matrix_options, runtime_options);
    offscreen_canvas = matrix->CreateFrameCanvas();
    
    matrix_width = matrix_options.cols * matrix_options.chain_length;
    matrix_height = matrix_options.rows * matrix_options.parallel;
    
    skipx = GAMEBOY_WIDTH / (GAMEBOY_WIDTH - matrix_width);
    skipy = GAMEBOY_HEIGHT / (GAMEBOY_HEIGHT - matrix_height);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "Matrix Width: %d Matrix Height: %d", matrix_width, matrix_height);
    Log(buffer);
    matrix->Clear();
}



void end_matrix(void){
    matrix->Clear();
    delete matrix;
}

void init_sdl()
{

    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,"1"); // Get Joystick Events without a focused window!

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) < 0)
    {
        Log("SDL Error Init: %s", SDL_GetError());
    }
 
    screen_width = GAMEBOY_WIDTH;
    screen_height = GAMEBOY_HEIGHT;

    if(!noWindow){
        SDL_CreateWindowAndRenderer(screen_width, screen_height, 0, &theWindow, &theRenderer);

        if (theWindow == NULL)
        {
            Log("SDL Error Video: %s", SDL_GetError());
        }

        theScreen =  SDL_CreateTexture(theRenderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STATIC, GAMEBOY_WIDTH, GAMEBOY_HEIGHT);

        SDL_ShowCursor(SDL_DISABLE);
    }

    game_pad = SDL_JoystickOpen(0);

    if(game_pad == NULL)
    {
        Log("Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError());
    }

    kc_keypad_left = SDLK_LEFT;
    kc_keypad_right = SDLK_RIGHT;
    kc_keypad_up = SDLK_UP;
    kc_keypad_down = SDLK_DOWN;
    kc_keypad_a = SDLK_a;
    kc_keypad_b = SDLK_s;
    kc_keypad_start = SDLK_RETURN;
    kc_keypad_select = SDLK_SPACE;
    kc_emulator_pause = SDLK_p;
    kc_emulator_quit = SDLK_ESCAPE;

    jg_x_axis_invert = false;
    jg_y_axis_invert = false;
    jg_a = 1;
    jg_b = 2;
    jg_start = 9;
    jg_select = 8;
    jg_x_axis = 0;
    jg_y_axis = 1;

    dmg_palette[0].red = 0xEF;
    dmg_palette[0].green = 0xF3;
    dmg_palette[0].blue = 0xD5;
    dmg_palette[0].alpha = 0xFF;

    dmg_palette[1].red = 0xA3;
    dmg_palette[1].green = 0xB6;
    dmg_palette[1].blue = 0x7A;
    dmg_palette[1].alpha = 0xFF;

    dmg_palette[2].red = 0x37;
    dmg_palette[2].green = 0x61;
    dmg_palette[2].blue = 0x3B;
    dmg_palette[2].alpha = 0xFF;

    dmg_palette[3].red = 0x00;      // was 0x04
    dmg_palette[3].green = 0x00;    // was 0x1C
    dmg_palette[3].blue = 0x00;     // was 0x16
    dmg_palette[3].alpha = 0xFF;

    if(!noWindow){
        SDL_SetRenderDrawColor(theRenderer, dmg_palette[0].red, dmg_palette[0].green, dmg_palette[0].blue, dmg_palette[0].alpha);
        SDL_RenderClear(theRenderer);
    }

    Config cfg;

    try
    {
        cfg.readFile(output_file);

        try
        {
            const Setting& root = cfg.getRoot();
            const Setting &gearboy = root["Gearboy"];

            string keypad_left, keypad_right, keypad_up, keypad_down, keypad_a, keypad_b,
            keypad_start, keypad_select, emulator_pause, emulator_quit;
            gearboy.lookupValue("keypad_left", keypad_left);
            gearboy.lookupValue("keypad_right", keypad_right);
            gearboy.lookupValue("keypad_up", keypad_up);
            gearboy.lookupValue("keypad_down", keypad_down);
            gearboy.lookupValue("keypad_a", keypad_a);
            gearboy.lookupValue("keypad_b", keypad_b);
            gearboy.lookupValue("keypad_start", keypad_start);
            gearboy.lookupValue("keypad_select", keypad_select);

            gearboy.lookupValue("joystick_gamepad_a", jg_a);
            gearboy.lookupValue("joystick_gamepad_b", jg_b);
            gearboy.lookupValue("joystick_gamepad_start", jg_start);
            gearboy.lookupValue("joystick_gamepad_select", jg_select);
            gearboy.lookupValue("joystick_gamepad_x_axis", jg_x_axis);
            gearboy.lookupValue("joystick_gamepad_y_axis", jg_y_axis);
            gearboy.lookupValue("joystick_gamepad_x_axis_invert", jg_x_axis_invert);
            gearboy.lookupValue("joystick_gamepad_y_axis_invert", jg_y_axis_invert);

            gearboy.lookupValue("emulator_pause", emulator_pause);
            gearboy.lookupValue("emulator_quit", emulator_quit);

            gearboy.lookupValue("emulator_pal0_red", dmg_palette[0].red);
            gearboy.lookupValue("emulator_pal0_green", dmg_palette[0].green);
            gearboy.lookupValue("emulator_pal0_blue", dmg_palette[0].blue);
            gearboy.lookupValue("emulator_pal1_red", dmg_palette[1].red);
            gearboy.lookupValue("emulator_pal1_green", dmg_palette[1].green);
            gearboy.lookupValue("emulator_pal1_blue", dmg_palette[1].blue);
            gearboy.lookupValue("emulator_pal2_red", dmg_palette[2].red);
            gearboy.lookupValue("emulator_pal2_green", dmg_palette[2].green);
            gearboy.lookupValue("emulator_pal2_blue", dmg_palette[2].blue);
            gearboy.lookupValue("emulator_pal3_red", dmg_palette[3].red);
            gearboy.lookupValue("emulator_pal3_green", dmg_palette[3].green);
            gearboy.lookupValue("emulator_pal3_blue", dmg_palette[3].blue);

            kc_keypad_left = SDL_GetKeyFromName(keypad_left.c_str());
            kc_keypad_right = SDL_GetKeyFromName(keypad_right.c_str());
            kc_keypad_up = SDL_GetKeyFromName(keypad_up.c_str());
            kc_keypad_down = SDL_GetKeyFromName(keypad_down.c_str());
            kc_keypad_a = SDL_GetKeyFromName(keypad_a.c_str());
            kc_keypad_b = SDL_GetKeyFromName(keypad_b.c_str());
            kc_keypad_start = SDL_GetKeyFromName(keypad_start.c_str());
            kc_keypad_select = SDL_GetKeyFromName(keypad_select.c_str());

            kc_emulator_pause = SDL_GetKeyFromName(emulator_pause.c_str());
            kc_emulator_quit = SDL_GetKeyFromName(emulator_quit.c_str());
        }
        catch(const SettingNotFoundException &nfex)
        {
            std::cerr << "Setting not found" << std::endl;
        }
    }
    catch(const FileIOException &fioex)
    {
        Log("I/O error while reading file: %s", output_file);
    }
    catch(const ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
            << " - " << pex.getError() << std::endl;
    }
}

void init(int argc, char** argv)
{
    init_sdl();
    init_matrix(argc, argv);
}

void end(void)
{
    end_matrix();
    SDL_JoystickClose(game_pad);
    SDL_DestroyWindow(theWindow);
    SDL_Quit();
}
// return the filenames of all files that have the specified extension
// in the specified directory and all subdirectories
void get_all(const fs::path& root, const std::vector<string> &ext, vector<fs::path>& ret)
{
    if(!fs::exists(root) || !fs::is_directory(root)) return;

    fs::recursive_directory_iterator it(root);
    fs::recursive_directory_iterator endit;

    while(it != endit)
    {
        
        if(fs::is_regular_file(*it)){
            string test = (it->path().extension().c_str());
            bool isThere = (std::find(ext.begin(), ext.end(), test) != ext.end());
            if(isThere){
                ret.push_back(it->path().filename());
            }
        }
        ++it;

    }

}
int main(int argc, char** argv)
{
    if (argc < 2)
    {
        printf("usage: %s rom_path [options]\n", argv[0]);
        printf("options:\n-nosound\n-forcedmg\n");
        return -1;
    }
        bool forcedmg = false;
    if (argc > 2)
    {
        for (unsigned int i = 2; i < argc; i++)
        {

            if (strcmp("-nowindow", argv[i]) == 0)
            {
                noWindow = true;
            }
        }
    }

    init(argc, argv);

    fs::path destination (argv[1]); //Works, no compiler error now
    vector<fs::path> ret;
    std::vector<std::string> validEndings {".gb", ".gbc"};
    get_all(destination, validEndings, ret);
    printf("roms found: %d \n", ret.size());
    for (int i=0; i<ret.size(); i++) {
        printf("Rom:: %s\n", ret[i].c_str());
    }
    running = false;
    while (running)
    {
        update();
    }

    end();

    return 0;
}
