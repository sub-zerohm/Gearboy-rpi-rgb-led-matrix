#include <stdio.h>
#include <stdlib.h>
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
#include "sromb.h"
#include "glcdfont.h"

using namespace std;
using namespace libconfig;
using namespace rgb_matrix;
namespace fs = ::boost::filesystem;

#define SCREEN_FPS 59.7
const uint32_t SCREEN_TICKS_PER_FRAME = 1000 / SCREEN_FPS; // (uint32_t) round()

bool running = true;
bool paused = false;

static const char *output_file = "/home/pi/gearboy.cfg";

bool audioEnabled = true;
vector<fs::path> romList;

uint8_t demoMode = 0;
vector<string> demoRomList;
uint32_t demoWaitTime = 10000;
uint32_t demoWaitLastInput = 0;


unsigned int argc;
char** argv;

string romFolder = "./";

palette_color dmg_palette[4];

unsigned int currentIndex = 0;

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

uint8_t scrollIndicator = 0;
uint32_t tickLastScroll = 0;
uint32_t scollTickDelay = 250;
uint8_t maxLetters = 16;

/*
 *  From: https://github.com/adafruit/rpi-fb-matrix/blob/master/display-test.cpp
 *  GNU V2 License: https://github.com/adafruit/rpi-fb-matrix/blob/master/LICENSE
 */ 
void printTextToCanvas(Canvas* canvas, int x, int y, const string& message,
                int r = 255, int g = 255, int b = 255) {
    // Loop through all the characters and print them starting at the provided
    // coordinates.
    for (auto c: message) {
        // Loop through each column of the character.
        for (int i=0; i<5; ++i) {
            unsigned char col = glcdfont[c*5+i];
            x += 1;
            // Loop through each row of the column.
            for (int j=0; j<8; ++j) {
                // Put a pixel for each 1 in the column byte.
                if ((col >> j) & 0x01) {
                    canvas->SetPixel(x, y+j, r, g, b);
                }
            }
        }
        // Add a column of padding between characters.
        x += 1;
    }
}

void fill_area(int x, int y, int width, int height, uint8_t r, uint8_t g, uint8_t b){
    for(int i=x; i<=x+width; i++){
        for(int j=y; j<=y+height; j++){
            offscreen_canvas->SetPixel(i,j, r,g,b);
        }
    }
}

void update_matrix(void){
        offscreen_canvas->Fill(dmg_palette[3].red, dmg_palette[3].green, dmg_palette[3].blue); // clear
        printTextToCanvas(offscreen_canvas, 25, 4,  "<GAME SELECT>");
        unsigned int startValue = ((int)currentIndex - 4 < 0 ? 0 : currentIndex - 4);
        //Log("Start value is %d",startValue);
        for (unsigned int i=startValue; i < startValue+10 && i < romList.size(); i++) {
            string filename =  romList[i].c_str();
            bool isGbcRom = false;
            if (filename.find(".gbc") != std::string::npos) {
                isGbcRom = true;
            }
            size_t lastindex = filename.find_last_of("."); 
            string cleanName = filename.substr(0, lastindex); 
            char buffer [50];
            if(i==currentIndex){
                fill_area(0,14+(i-startValue)*10, GAMEBOY_WIDTH, 8, dmg_palette[2].red, dmg_palette[2].green, dmg_palette[2].blue);
                
                if(cleanName.size() > maxLetters){
                    uint8_t nextScrollIndicator = ((scrollIndicator+1) % ((cleanName.size()-maxLetters)+1));
                    if(SDL_GetTicks()-tickLastScroll >  ((scrollIndicator==0 || nextScrollIndicator == 0)? scollTickDelay*4 : scollTickDelay)){
                        //scrollIndicator = ((scrollIndicator+1) % ((cleanName.size()/maxLetters)+1));
                        scrollIndicator = nextScrollIndicator;
                        tickLastScroll = SDL_GetTicks();
                        Log("Scrolling Text  %d",scrollIndicator);
                    }
                    //string partial = cleanName.substr(scrollIndicator*maxLetters);
                    string partial = cleanName.substr(scrollIndicator, std::min((int)maxLetters, (int)(cleanName.size()-scrollIndicator)));
                    sprintf(buffer, "%s%s", (isGbcRom? "GBC ":"GB  "),partial.c_str());
                }else{
                    sprintf(buffer, "%s%s", (isGbcRom? "GBC ":"GB  "),cleanName.c_str());
                }
                
            }else{
                string partial = cleanName.substr(0, std::min((int)maxLetters, (int)(cleanName.size())));
                sprintf(buffer, "%s%s", (isGbcRom? "GBC ":"GB  "),partial.c_str());
            }
            printTextToCanvas(offscreen_canvas, 2, 15+(i-startValue)*10,  buffer);
        }
        if(demoMode == 1){
            printTextToCanvas(offscreen_canvas, 2, matrix_height-12,  "D"); // Demo Mode (no Audio unless used)
        }else if(demoMode == 2){
            printTextToCanvas(offscreen_canvas, 2, matrix_height-12,  "A"); // Attract Mode (Audio during demo)
        }
    offscreen_canvas = matrix->SwapOnVSync(offscreen_canvas);
}

void update(void)
{
    
    timer_fps_cap_ticks_start = SDL_GetTicks();
    SDL_Event keyevent;

    
    if(SDL_JoystickGetButton(game_pad, 2) && SDL_JoystickGetButton(game_pad, 5) && SDL_JoystickGetButton(game_pad, jg_select) && SDL_JoystickGetButton(game_pad, jg_b)){ // Press start and Select to Exit
        running = false;
        end();
        system("sudo shutdown -hP now");
        return;
    }


    if(demoMode > 0 && demoWaitLastInput + demoWaitTime < SDL_GetTicks()){
        if(demoRomList.size() < 1){
            demoMode = 0;
            return;
        }
        int randomIndex = rand() % demoRomList.size();
        runRom(string(demoRomList[randomIndex].c_str()), true);
        demoWaitLastInput = SDL_GetTicks();
        return;
    }

    while (SDL_PollEvent(&keyevent))
    {
        switch(keyevent.type)
        {
            case SDL_QUIT:
            running = false;
            Log("EXITING!!!");
            break;

            case SDL_JOYBUTTONDOWN:
            {
                demoWaitLastInput = SDL_GetTicks();
                if (keyevent.jbutton.button == jg_b){
                    
                }
                else if (keyevent.jbutton.button == jg_a){
                    runRom(romList[currentIndex].c_str());
                }
                else if (keyevent.jbutton.button == jg_select){

                }
                else if (keyevent.jbutton.button == jg_start){
                    runRom(romList[currentIndex].c_str());
                }else if (keyevent.jbutton.button == 5){
                    demoMode = (demoMode+1) % 3;
                    Log("DemoMode: "+ demoMode);
                }
 
            }
            break;

            case SDL_JOYBUTTONUP:
            {
                if (keyevent.jbutton.button == jg_b){

                }
                else if (keyevent.jbutton.button == jg_a){

                }
                else if (keyevent.jbutton.button == jg_select){

                }
                else if (keyevent.jbutton.button == jg_start){

                }

            }
            break;

            
            case SDL_JOYAXISMOTION:
            {
                demoWaitLastInput = SDL_GetTicks();
                if(keyevent.jaxis.axis == jg_x_axis)
                {
                    int x_motion = keyevent.jaxis.value * (jg_x_axis_invert ? -1 : 1);
                    if (x_motion < 0){

                    }
                    else if (x_motion > 0){
                    
                    }
                    else
                    {

                    }
                }
                else if(keyevent.jaxis.axis == jg_y_axis)
                {
                    int y_motion = keyevent.jaxis.value * (jg_y_axis_invert ? -1 : 1);
                    if (y_motion < -3200){
                        if(currentIndex == 0){
                            currentIndex = romList.size()-1;
                        }else{
                            currentIndex -= 1;
                        }
                        Log("Current Index %d", currentIndex);
                        tickLastScroll = SDL_GetTicks();
                        scrollIndicator = 0;
                    }
                    else if (y_motion > 3200){
                        currentIndex += 1;
                        currentIndex %= romList.size();
                        Log("Current Index %d", currentIndex);
                        tickLastScroll = SDL_GetTicks();
                        scrollIndicator = 0;
                    }
                    else
                    {

                    }
                }
            }
            break;
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


size_t split(const std::string &txt, std::vector<std::string> &strs, char ch)
{
    size_t pos = txt.find( ch );
    size_t initialPos = 0;
    strs.clear();

    // Decompose statement
    while( pos != std::string::npos ) {
        strs.push_back( txt.substr( initialPos, pos - initialPos ) );
        initialPos = pos + 1;

        pos = txt.find( ch, initialPos );
    }

    // Add the last one
    strs.push_back( txt.substr( initialPos, std::min( pos, txt.size() ) - initialPos + 1 ) );

    return strs.size();
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

    dmg_palette[0].red = 0xCD;      // was EF
    dmg_palette[0].green = 0xD6;    // was F3
    dmg_palette[0].blue = 0xBB;     // was D5
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

            const Setting &sromb = root["Sromb"];
            string demoRomListString;
            sromb.lookupValue("demoRomList", demoRomListString);
            if(demoRomListString.size() > 0){
                int size = (int)split(demoRomListString, demoRomList, ',' );
                Log("Found Demo Playlist Size: %d", size);
            }


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
    if(demoRomList.size() == 0){
        Config cfg;

        Setting &root = cfg.getRoot();
        Setting &address = root.add("Sromb", Setting::TypeGroup);
        address.add("demoRomList", Setting::TypeString) = "";

        try
        {
            cfg.writeFile(output_file);
        }
        catch(const FileIOException &fioex)
        {
            Log("I/O error while writing file: %s", output_file);
        }
    }

    end_matrix();
    SDL_JoystickClose(game_pad);
    SDL_DestroyWindow(theWindow);
    SDL_Quit();
}

void runRom(string romFile, bool isAutoStart){
    end();
    char buffer[512];
    string args = "";
    for(unsigned int i=0; i < argc; i++){

        args += " "+string(argv[i]);
    }
    sprintf(buffer, "gearboymatrix \"%s%s\" %s%s%s", romFolder.c_str(), romFile.c_str(), (isAutoStart? "-demo ":(demoMode != 0?"-autoexit ":"")),(demoMode==1?"-mutedemo ": ""), (args.c_str()));
    Log("Executing: %s", buffer);
    system(buffer);
    usleep(500);
    init(argc, argv);
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
                romList.push_back(it->path().filename());
            }
        }
        ++it;

    }

}

int main(int pArgc, char** pArgv)
{
    argc = pArgc;
    argv = pArgv;

    if (argc < 2)
    {
        printf("usage: %s rom_path [options]\n", argv[0]);
        printf("options:\n-nosound\n-forcedmg\n");
        return -1;
    }
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
    romFolder = argv[1];
    printf("roms dir: %s \n", romFolder.c_str());
    std::vector<std::string> validEndings {".gb", ".gbc"};
    get_all(destination, validEndings, romList);
    sort(romList.begin(),romList.end());
    printf("roms found: %d \n", romList.size());
    for (unsigned int i=0; i<romList.size(); i++) {
        printf("Rom:: %s\n", romList[i].c_str());
    }
    
    while (running)
    {
        update();
    }

    end();

    return 0;
}
