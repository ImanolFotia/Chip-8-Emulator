/*
MIT License

Copyright (c) 2018 Imanol Fotia

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include <memory>
#include <iostream>
#include <mutex>
#include <vector>
#include <map>
#include <string>
#include <chrono>
#include <stack>
#include <random>
#include <functional>
#include <cstring>
#include <fstream>
#include <bitset>
#include <set>
#include <thread>
#include <chrono>

//SDL is a 3rd party library that can create windows, poll events, audio, etc
#include <SDL2/SDL.h>

#define NOP 0

typedef uint8_t   U8;
typedef uint16_t  U16;
typedef uint32_t  U32;
typedef double_t  F64;
typedef void    U0;


typedef std::shared_ptr<U8>   U8_ptr;
typedef std::shared_ptr<U16>  U16_ptr;
typedef std::shared_ptr<U32>  U32_ptr;


namespace REGISTERS
{
    constexpr U32 MAX_V_REGS = 16;

    enum REGS{V0 = 0x0, V1, V2, V3, V4, V5, V6, V7, V8, V9, VA, VB, VC, VD, VE, VF};

    U8 VREGISTERS[MAX_V_REGS] = {0};

    U16 I = 0x0;
    
    static U0 init_registers(){}
}

//Chip-8 only uses a 16-key keyboard as input device (from 0 to F)
//These keys can be mapped in any way comfortable to the user
namespace IO{
    enum KEYS {K0 = 0x00, K1, K2, K3, K4, K5, K6, K7, K8, K9, KA, KB, KC, KD, KE, KF};
    bool KEYBOARD[16] = {false};

    using key_map = std::map<int, KEYS>;
    key_map key_bindings = { {SDL_SCANCODE_KP_0, K0 }, {SDL_SCANCODE_KP_1, K1 }, {SDL_SCANCODE_KP_2, K2 }, 
                             {SDL_SCANCODE_KP_3, K3 }, {SDL_SCANCODE_KP_4, K4 }, {SDL_SCANCODE_KP_5, K5 }, 
                             {SDL_SCANCODE_KP_6, K6 }, {SDL_SCANCODE_KP_7, K7 }, {SDL_SCANCODE_KP_8, K8 }, 
                             {SDL_SCANCODE_KP_9, K9 }, {SDL_SCANCODE_A,    KA }, {SDL_SCANCODE_S,    KB }, 
                             {SDL_SCANCODE_D,    KC }, {SDL_SCANCODE_F,    KD }, {SDL_SCANCODE_C,    KE },
                             {SDL_SCANCODE_V,    KF },    
                            };
                            
    #define k_e event.key.keysym.scancode
    U0 poll_events(){
        //for(auto k: key_bindings)KEYBOARD[k.second] = false;
        SDL_Event event;
        SDL_PollEvent(&event);
        for(auto k: key_bindings){if(k.first==k_e)KEYBOARD[k.second] = (event.type == SDL_KEYDOWN) ? true : false ;}
    }
}

//DISPLAY
//Chip-8 uses a custom font consisting of "Sprite" like characters
//The maximum size for a sprite is 15 bytes, for a possible sprite size of 8x15
namespace DISPLAY
{

    U32 DISPLAY_WIDTH = 64;
    U32 DISPLAY_HEIGHT = 32;
    U16 VRAM_SIZE = 2048;
    bool UPDATE = false;
    enum MODE {LOW = 0, HIGH};
    U8 CURRENT_MODE = LOW;
    //Sprite structure
    //Each character is represented by 5 U8 rows
    /**
     * For example number 0 would be represented as follows:
     * 
        "0"	  Binary  Hex
        **** 11110000 0xF0
        *  * 10010000 0x90
        *  * 10010000 0x90
        *  * 10010000 0x90
        **** 11110000 0xF0
    */

    enum CHARACTER {C0=0,C1,C2,C3,C4,C5,C6,C7,C8,C9,CA,CB,CC,CD,CE,CF};

    //Font character used for the original CHIP-8
    //size 640 bytes
    U8 c8_font[16*5] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xF0, 0x90, 0x90, 0x90, 0xF0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };

    U8 sc8_font[16*16] = { 
        0x3c, 0x66, 0xc3, 0x81, 0x81,
        0x81, 0x81, 0xc3, 0x66, 0x3c, 
        0x10, 0x30, 0x10, 0x10, 0x10,
        0x10, 0x10, 0x10, 0x10, 0x7e,
        0x3c, 0x66, 0x81, 0x01, 0x03,
        0x06, 0x38, 0xc0, 0x80, 0xff,
        0x7e, 0x83, 0x81, 0x01, 0x06,
        0x06, 0x01, 0x81, 0x83, 0x7e,
        0x04, 0x0c, 0x14, 0x24, 0x44,
        0xff, 0x04, 0x04, 0x04, 0x04,
        0xff, 0x80, 0x80, 0x80, 0xfe,
        0xc2, 0x01, 0x01, 0x83, 0x7e,
        0x3e, 0x41, 0x81, 0x80, 0xbc,
        0xc2, 0x81, 0x81, 0x81, 0x7e,
        0xff, 0x01, 0x03, 0x06, 0x08,
        0x10, 0x20, 0x60, 0x40, 0x80,
        0x18, 0x24, 0x42, 0x42, 0x3c,
        0x42, 0x81, 0x81, 0x81, 0x7e,
        0x7e, 0x81, 0x81, 0x81, 0x43,
        0x3d, 0x01, 0x81, 0x86, 0x7c,
        0x18, 0x66, 0x42, 0x81, 0x81,
        0xff, 0x81, 0x81, 0x81, 0x81, 
        0xfc, 0x82, 0x82, 0x82, 0xfc,
        0x82, 0x81, 0x81, 0x81, 0xfe,
        0x3c, 0x66, 0xc3, 0x81, 0x80,
        0x80, 0x81, 0xc3, 0x66, 0x3c,
        0xf8, 0x84, 0x82, 0x81, 0x81,
        0x81, 0x81, 0x82, 0x84, 0xf8,
        0xff, 0x80, 0x80, 0x80, 0x80,
        0xfc, 0x80, 0x80, 0x80, 0xff,
        0xff, 0x80, 0x80, 0x80, 0x80,
        0xfc, 0x80, 0x80, 0x80, 0x80,  
    };

    U8* VRAM = nullptr;

    SDL_Window* SDL_window = nullptr;
    SDL_Renderer *SDL_renderer = nullptr;
    SDL_Texture *SDL_texture = nullptr;
    SDL_Surface *SDL_surface = nullptr;


    U0 update_display(){
        if(UPDATE) {
            SDL_SetRenderDrawColor(SDL_renderer, 0x00, 0xFF, 0x00, 0xFF);
            U8* pixels = new U8[VRAM_SIZE * 4];
            for(int i = 0 ;i < VRAM_SIZE; i++)for(int j = 0; j < 4; j++)pixels[(i*4) + j] = VRAM[i];
            SDL_UpdateTexture(SDL_texture, nullptr, pixels, sizeof (U8) * DISPLAY_WIDTH * 4);
            SDL_RenderCopy(SDL_renderer, SDL_texture, nullptr, nullptr);
            SDL_RenderPresent(SDL_renderer);
            UPDATE = false;
        }
    }

    static U0 create_window()
    {
        SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO ); 
        SDL_window =  SDL_CreateWindow("Chip-8 Emulator by Imanol Fotia", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL );

        if (SDL_window == NULL) {
            printf("Could not create window: %s\n", SDL_GetError());
            return;
        }
        
    }

    static U0 reset(MODE m){
        CURRENT_MODE = m;
        DISPLAY_WIDTH = 64 * (m + 0x1);
        DISPLAY_HEIGHT = 32 * (m + 0x1);

        VRAM_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
        delete VRAM;
        VRAM = nullptr;
        VRAM = new U8[VRAM_SIZE];
        for(int i = 0; i < DISPLAY::VRAM_SIZE; i++) DISPLAY::VRAM[i] = 0x00;
        SDL_texture = SDL_CreateTexture(SDL_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
    }

    static U0 init_display(){
        VRAM_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
        VRAM = new U8[VRAM_SIZE];
        for(int i = 0; i < DISPLAY::VRAM_SIZE; i++) DISPLAY::VRAM[i] = 0x00;
        create_window();
        
        SDL_renderer = SDL_CreateRenderer(SDL_window, -1, SDL_RENDERER_ACCELERATED);
        SDL_texture = SDL_CreateTexture(SDL_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, DISPLAY_WIDTH, DISPLAY_HEIGHT);
        
    }
} // namespace DISPLAY

//Chip-8 can access 4KB(4096 bytes) of RAM
//The first 512 bytes are reserved for the interpreter
namespace MEMORY
{
    U8* RAM = nullptr;

    constexpr U32 MAX_RAM = 0x1000;
    constexpr U16 PROGRAM_SPACE = 0x200;
    constexpr U16 RESERVED = 0x1FF;
    constexpr U16 MAX_PROGRAM_SIZE = 0x0E00;

    static U0 init_memory(U8* ROM){
        MEMORY::RAM = new U8[MAX_RAM];
        std::memcpy(RAM, DISPLAY::c8_font, sizeof(U8) * 16 * 5);

        std::memcpy(&RAM[0] + PROGRAM_SPACE, ROM, sizeof(U8) * MAX_PROGRAM_SIZE);
    }
}

// Chip-8 specification has 2 timers
// A delay timer (DT) and a sound timer (ST)
// 
namespace TIMERS
{
    F64 delta_time = 0.0;
    F64 last_time = 0.0;
    F64 current_time = 0.0;
    F64 time_acc = 0.0;
    //The delay timer is active whenever it's value in not zero, it decreases at 60hz.
    size_t delay_timer = 0;
    
    //The sound timer is active whenever it's value in not zero, it decreases at 60hz.
    //this means that while ST > 0 the buzzer will sound
    size_t sound_timer = 0;

    static U0 update_timers()
    {
        current_time = SDL_GetTicks();
        delta_time = current_time - last_time;
        last_time = current_time;

        time_acc += delta_time;

        if(delay_timer > 0) delay_timer -= delta_time * 0.016;
        if(sound_timer > 0) sound_timer -= delta_time * 0.016;
    }
    
    static U0 init_timers(){}
} 

namespace SYSTEM{

    static U8* load_file(const char* program){
        std::ifstream f(program, std::ios::binary);
        if(!f.is_open())
        {
            std::clog << "Couldn't open program\n"; return nullptr;
        }
        U8* ROM = new U8[MEMORY::MAX_PROGRAM_SIZE];

        f.seekg(0, std::ios::beg);
        f.read((char*)ROM, MEMORY::MAX_PROGRAM_SIZE);
        f.close();

        return ROM;
    }

    bool shift_quirks = false;
    bool load_store_quirks = false;
    bool screen_wraping = false;
    bool debug_execution = false;
}

//OPCODES
//These represent all possible instructions in a program
//36 of these instructions belong to the original CHIP-8 implementation, while the remaining 10 belong to Super Chip-48
//All instructions are 2 bytes long and are stored in the first byte
namespace CPU
{

    #define regs REGISTERS::VREGISTERS
    #define r REGISTERS::REGS
    #define rI REGISTERS::I
    #define lfunc [&](U32 x, U32 y, U8 kk, U16 nnn, U8 n) -> U0

    //Program Counter stores currently executing address
    U16 PC = MEMORY::PROGRAM_SPACE;
    //Stack pointer points to the top of the stack
    U8 SP = 0;

    std::stack<U16> STACK;

    auto SYS =     lfunc {PC += 0x0002;}; //Jump to a machine code routine at nnn.
    auto CLS =     lfunc {for(int i = 0; i < DISPLAY::VRAM_SIZE; i++) DISPLAY::VRAM[i] = 0x00; DISPLAY::UPDATE = true; PC += 0x0002;}; //Clear the screen (00E0)
    auto RET =     lfunc {PC = STACK.top(); STACK.pop(); SP--;PC += 0x0002;}; //Return from subroutine (00EE)
    auto JP_1 =    lfunc {PC = nnn & 0x0FFF;}; //Jump to location nnn.
    auto CALL =    lfunc {SP++;STACK.push(PC);PC = nnn & 0x0FFF;}; //Jump to machine code routine nnn
    auto SE_3 =    lfunc {(regs[x] == (kk & 0x00FF) ? PC+=0x0004 : PC += 0x0002);}; //Skip next instruction if Vx = kk.
    auto SNE_4 =   lfunc {(regs[x] != (kk & 0x00FF) ? PC+=0x0004 : PC += 0x0002);}; //Skip next instruction if Vx != kk.
    auto SE_5 =    lfunc {(regs[x] == regs[y] ? PC+=0x0004 : PC += 0x0002);}; //Skip next instruction if Vx = Vy.
    auto LD_6 =    lfunc {regs[x] = kk & 0x00FF; PC += 0x0002;}; //Set Vx = kk.
    auto ADD_7 =   lfunc {regs[x] += (kk & 0x00FF); PC += 0x0002;}; //Set Vx = Vx + kk.
    auto LD_8 =    lfunc {regs[x] = regs[y]; PC += 0x0002;}; //Set Vx = Vy.
    auto OR =      lfunc {regs[x] |= regs[y]; PC += 0x0002;}; //Set Vx = Vx OR Vy.
    auto AND =     lfunc {regs[x] &= regs[y]; PC += 0x0002;}; //Set Vx = Vx AND Vy.
    auto XOR =     lfunc {regs[x] ^= regs[y]; PC += 0x0002;}; //Set Vx = Vx XOR Vy. 
    auto ADD_8 =   lfunc {U16 res = regs[x] + regs[y];regs[r::VF] = (res >= 0x0100) ? 0x01 : 0x00;regs[x] = res & 0x00FF; PC += 0x0002;}; //Set Vx = Vx + Vy, set VF = carry.
    auto SUB =     lfunc {regs[r::VF] = regs[x] > regs[y] ? 0x01 : 0x00;regs[x] = regs[x] - regs[y]; PC += 0x0002;}; //Set Vx = Vx - Vy, set VF = NOT borrow.
    auto SHR =     lfunc {regs[r::VF] = (regs[x] & 0x01 ); SYSTEM::shift_quirks ? regs[x]>>=1 : regs[x] = regs[y] >> 0x1; PC += 0x0002;}; //Set Vx = Vx SHR 1.
    auto SUBN =    lfunc {regs[r::VF] = regs[x] < regs[y] ? 0x01 : 0x00;regs[x] = regs[y] - regs[x]; PC += 0x0002;}; //Set Vx = Vy - Vx, set VF = NOT borrow.
    auto SHL =     lfunc {regs[r::VF] = (regs[x] >> 7 & 0x01);SYSTEM::shift_quirks ? regs[x] <<= 0x01 : regs[x] = regs[y] << 0x1; PC += 0x0002;}; //Set Vx = Vx SHL 1.
    auto SNE_9 =   lfunc {regs[x] != regs[y] ? PC+=0x0004 : PC += 0x0002;}; //Skip next instruction if Vx != Vy.
    auto LD_A =    lfunc {rI = (nnn) & 0x0FFF; PC += 0x0002;}; //Set I = nnn.
    auto JP_B =    lfunc {PC = (nnn + regs[r::V0]) & 0x0FFF;}; //Jump to location nnn + V0.
    auto RND =     lfunc {regs[x] = rand() & kk; PC += 0x0002;}; //Set Vx = random byte AND kk.
    auto DRW =     lfunc {regs[r::VF] = 0x00;for(U16 i = 0; i < n; i++){U8 pixel = MEMORY::RAM[rI + i];for(U16 j = 0; j < 8; j++){U8 px = (pixel & (0x80 >> j));
                   U8* old_px = &DISPLAY::VRAM[  ( (regs[x] + j) + ((i + regs[y]) * DISPLAY::DISPLAY_WIDTH)) % (SYSTEM::screen_wraping ? DISPLAY::VRAM_SIZE : 0xFFFF)];if(px != 0x0){if(*old_px != 0x0)regs[r::VF] = 0x01;
                   *old_px ^= 0xFF;}}}DISPLAY::UPDATE = true;PC += 0x0002;}; //Display n-byte sprite starting at memory location I at (Vx, Vy), set VF = collision.
    auto SKP =     lfunc {PC += (IO::KEYBOARD[regs[x]]) == true ? 0x0004 : 0x0002;}; //Skip next instruction if key with the value of Vx is pressed.
    auto SKNP =    lfunc {PC += (IO::KEYBOARD[regs[x]]) != true ? 0x0004 : 0x0002;}; //Skip next instruction if key with the value of Vx is not pressed.
    auto LD_F7 =   lfunc {regs[x] = TIMERS::delay_timer & 0x0F; PC += 0x0002;}; //Set Vx = delay timer value.
    auto LD_FA =   lfunc {unsigned i=0;while(!IO::KEYBOARD[i%16]){IO::poll_events();i++;} regs[x] = i%16; PC += 0x0002;}; //Wait for a key press, store the value of the key in Vx.
    auto LD_F15 =  lfunc {TIMERS::delay_timer = regs[x]; PC += 0x0002;}; //Set delay timer = Vx.
    auto LD_F18 =  lfunc {TIMERS::sound_timer = regs[x]; PC += 0x0002;}; //Set sound timer = Vx.
    auto ADD_F1E = lfunc {rI += regs[x]; PC += 0x0002;}; //Set I = I + Vx.
    auto LD_F29 =  lfunc {rI = (regs[x] * 0x0005); PC += 0x0002;}; //Set I = location of sprite for digit Vx.
    auto LD_F33 =  lfunc {MEMORY::RAM[rI] = (regs[x] / 100);MEMORY::RAM[rI+1] = (regs[x] / 10) % 10; MEMORY::RAM[rI+2] = (regs[x]%100)%10; PC += 0x0002;}; //Store BCD representation of Vx in memory locations I, I+1, and I+2.
    auto LD_F55 =  lfunc {for(U8 i = 0; i <= x; i++){MEMORY::RAM[rI + i] = regs[i];}SYSTEM::load_store_quirks ? rI += x + 1 : NOP;PC += 0x0002;}; //Store registers V0 through Vx in memory starting at location I. Vx INCLUDED!!
    auto LD_F65 =  lfunc {for(U8 i = 0; i <= x; i++){regs[i] = MEMORY::RAM[rI + i];}SYSTEM::load_store_quirks ? rI += x + 1 : NOP;PC += 0x0002;}; //Read registers V0 through Vx from memory starting at location I. Vx INCLUDED!!
    
    //The following are instructions for the CHIP-48
    //It is an extension to chip-8 original design
    auto SCU = lfunc {memmove(DISPLAY::VRAM, &DISPLAY::VRAM[16 * n], 1024 - 16 * n);memset(&DISPLAY::VRAM[1023 - 16 * n], 0, 16 * n);PC += 0x0002;}; //Scroll up N pixels (N/2 pixels in low res mode)
    auto SCD = lfunc {memmove(&DISPLAY::VRAM[16 * n], DISPLAY::VRAM, 1024 - 16 * n);memset(DISPLAY::VRAM, 0, 16 * n);PC += 0x0002;}; //Scroll down N pixels (N/2 pixels in low res mode)
    auto SCR = lfunc {PC += 0x0002; }; //Scroll right 4 pixels (2 pixels in low res mode)
    auto SCL = lfunc {PC += 0x0002;}; //Scroll left 4 pixels (2 pixels in low res mode)
    auto EXIT = lfunc {std::cout << "EXIT called" << std::endl;exit(0); }; //Exit the interpreter; this causes the VM to infinite loop
    auto LOW = lfunc {DISPLAY::reset(DISPLAY::MODE::LOW); PC += 0x0002;}; //Enter low resolution (64x32) mode; this is the default mode
    auto HIGH = lfunc {DISPLAY::reset(DISPLAY::MODE::HIGH); PC += 0x0002;}; //Enter high resolution (128x64) mode
    //auto DRW = lfunc {}; //Draw a 16x16 sprite at I to VX, VY (8x16 in low res mode) (*** see note)
    auto LD_HF = lfunc {rI = regs[x] * 0x08; PC += 0x0002;}; //I = address of 8x10 font character in VX (0..F) (* see note)
    auto LD_R = lfunc {std::cout << "LD_R called" << std::endl; PC += 0x0002;}; //Store V0..VX (inclusive) into HP-RPL user flags R0..RX (X < 8)
    auto LD_VX = lfunc {std::cout << "LD_VX called" << std::endl; PC += 0x0002;}; //Load V0..VX (inclusive) from HP-RPL user flags R0..RX (X < 8)

    struct opcode_t{
        opcode_t(U16 a, U8 b, U32 c, U32 d, U8 e, U16 f) 
        : nnn(a), kk(b), x(c), y(d), n(e), oc(f) {}
        U16 nnn;
        U8 kk;
        U32 x;
        U32 y;
        U8 n;
        U16 oc;
    };

    using function_map = std::map<U16, std::function<U0(U32, U32, U8, U16, U8)> >;
    using mnemonic_map = std::map<U16, std::string>;
    using opcode_set = std::set<U16>;
    // There are several types of opcodes, each type as to be & by a different value
    // type 0xX0X0 : 0xF0FF
    opcode_set opcode_type_1 = { 0x00E0, 0x00EE, 0x00FB, 0x00FC, 0x00FD, 0x00FE, 0x00FF, 0x0000 /*SYS is ignored*/, 
                                 0xE09E, 0xE0A1, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF030, 0xF033, 0xF055, 0xF065};
                                 
    // type 0xX00X : 0xF00F
    opcode_set opcode_type_2 = { 0x8001, 0x8002, 0x8003, 0x8004, 0x8005, 0x8006, 0x8007, 0x800E, 0xF007, 0xF00A};

    // type 0xX000 : 0xF000
    opcode_set opcode_type_3 = { 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7000, 0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000 };

    // type 0x00X0 : 0x00F0 (these are only used by Chip-48 instructions)
    opcode_set opcode_type_4 = { 0x00B0, 0x00C0, 0xF015, 0xF018, 0xF01E, 0xF029, 0xF033, 0xF055, 0xF065, 0xF075, 0xF085};

    function_map Execute { 
        { 0x00E0, CLS   }, { 0x00EE, RET   }, { 0x1000, JP_1   }, { 0x0000, SYS /* SYS is ignored by most programs*/ }, 
        { 0x2000, CALL  }, { 0x3000, SE_3  }, { 0x4000, SNE_4  }, { 0x5000, SE_5  }, 
        { 0x6000, LD_6  }, { 0x7000, ADD_7 }, { 0x8000, LD_8   }, { 0x8001, OR    }, 
        { 0x8002, AND   }, { 0x8003, XOR   }, { 0x8004, ADD_8  }, { 0x8005, SUB   }, 
        { 0x8006, SHR   }, { 0x8007, SUBN  }, { 0x800E, SHL    }, { 0x9000, SNE_9 }, 
        { 0xA000, LD_A  }, { 0xB000, JP_B  }, { 0xC000, RND    }, { 0xD000, DRW   }, 
        { 0xE09E, SKP   }, { 0xE0A1, SKNP  }, { 0xF007, LD_F7  }, { 0xF00A, LD_FA }, 
        { 0xF015, LD_F15}, { 0xF018, LD_F18}, { 0xF01E, ADD_F1E}, { 0xF029, LD_F29}, 
        { 0xF033, LD_F33}, { 0xF055, LD_F55}, { 0xF065, LD_F65 },
        //Chip-48 instructions
        { 0x00B0, SCU   }, { 0x00C0, SCD   }, { 0x00FB, SCR    }, { 0x00FC, SCL   },
        { 0x00FD, EXIT  }, { 0x00FE, LOW   }, { 0x00FF, HIGH   }, { 0xF030, LD_HF },
        { 0xF075, LD_R  }, { 0xF085, LD_VX }
    };

    mnemonic_map MNEMONICS = {
        { 0x00E0, "CLS"   }, { 0x00EE, "RET"   }, { 0x1000, "JP_1"   }, { 0x0000, "SYS" /* SYS is ignored by most programs*/ }, 
        { 0x2000, "CALL"  }, { 0x3000, "SE_3"  }, { 0x4000, "SNE_4"  }, { 0x5000, "SE_5"  }, 
        { 0x6000, "LD_6"  }, { 0x7000, "ADD_7" }, { 0x8000, "LD_8"   }, { 0x8001, "OR"    }, 
        { 0x8002, "AND"   }, { 0x8003, "XOR"   }, { 0x8004, "ADD_8"  }, { 0x8005, "SUB"   }, 
        { 0x8006, "SHR"   }, { 0x8007, "SUBN"  }, { 0x800E, "SHL"    }, { 0x9000, "SNE_9" }, 
        { 0xA000, "LD_A"  }, { 0xB000, "JP_B"  }, { 0xC000, "RND"    }, { 0xD000, "DRW"   }, 
        { 0xE09E, "SKP"   }, { 0xE0A1, "SKNP"  }, { 0xF007, "LD_F7"  }, { 0xF00A, "LD_FA" }, 
        { 0xF015, "LD_F15"}, { 0xF018, "LD_F18"}, { 0xF01E, "ADD_F1E"}, { 0xF029, "LD_F29"}, 
        { 0xF033, "LD_F33"}, { 0xF055, "LD_F55"}, { 0xF065, "LD_F65" },
        //Chip-48 instructions
        { 0x00B0, "SCU"   }, { 0x00C0, "SCD"   }, { 0x00FB, "SCR"    }, { 0x00FC, "SCL"   },
        { 0x00FD, "EXIT"  }, { 0x00FE, "LOW"   }, { 0x00FF, "HIGH"   }, { 0xF030, "LD_HF" },
        { 0xF075, "LD_R"  }, { 0xF085, "LD_VX" }
    };

    static U16 get_opcode_type_1(U16 opcode) { return opcode & 0xF0FF; }
    static U16 get_opcode_type_2(U16 opcode) { return opcode & 0xF00F; }
    static U16 get_opcode_type_3(U16 opcode) { return opcode & 0xF000; }
    static U16 get_opcode_type_4(U16 opcode) { return opcode & 0x00F0; }
    //static U16 get_opcode_type_5(U16 opcode) { return opcode & 0x00F0; }

    static U0 runtime_debug(opcode_t opcode)
    {   std::cout << std::hex << "PC = " << std::bitset<16>(PC - MEMORY::PROGRAM_SPACE) << std::endl;
        std::cout << "SP = " << std::bitset<8>(SP) << std::endl;
        //if(STACK.size() > 0)
        //    std::cout << "Top of the stack = " << std::bitset<16>(STACK.top()) << std::endl; 
        std::cout << "0x" << std::bitset<16>(PC).to_ullong() << " 0x" << std::hex << std::bitset<16>(opcode.oc).to_ullong()
                                                                                  << ": " << MNEMONICS[opcode.oc] << std::endl 
                                                                                  << " nnn = " << std::bitset<16>(opcode.nnn)
                                                                                  << " kk = " << std::bitset<8>(opcode.kk)
                                                                                  << " x = " << std::bitset<16>(opcode.x)
                                                                                  << " y = " << std::bitset<16>(opcode.y)
                                                                                  << " n = " << std::bitset<4>(opcode.n) << "\r" << std::endl;
    }

    static opcode_t fetch_opcode() {

        U16 opcode = MEMORY::RAM[PC] << 8 | MEMORY::RAM[PC + 1];
        U16 nnn = (opcode) & 0x0FFF;
        U8 kk =   (opcode) & 0x00FF;
        U16 x =   (opcode & 0x0F00) >> 8;
        U16 y =   (opcode & 0x00F0) >> 4;
        U8  n =   (opcode) & 0x000F;

        U16 op1 = get_opcode_type_1(opcode), op2 = get_opcode_type_2(opcode), op3 = get_opcode_type_3(opcode), op4 = get_opcode_type_4(opcode);/*, op5 = get_opcode_type_5(opcode);*/
        U16 out_opcode = 0x0000;

        if (opcode_type_1.find(op1) != opcode_type_1.end()) out_opcode = op1;
        else if (opcode_type_2.find(op2) != opcode_type_2.end()) out_opcode = op2;
        else if (opcode_type_3.find(op3) != opcode_type_3.end()) out_opcode = op3;
        else if (opcode_type_4.find(op4) != opcode_type_4.end()) out_opcode = op4;
        else {}
        if(SYSTEM::debug_execution)
        runtime_debug(opcode_t(nnn, kk, x, y, n, out_opcode));
        return opcode_t(nnn, kk, x, y, n, out_opcode);
        
    }
    

    static U0 emulate_cycle(opcode_t opcode)
    {
        try
        {   
            Execute.at(opcode.oc)(opcode.x, opcode.y, opcode.kk, opcode.nnn, opcode.n);
        }
        catch(std::exception e)
        {
            std::cout << e.what() << std::endl;
            PC += 2;
        }
    }

    static U0 init_cpu(){
        SP = 0;
    }
} // namespace CPU


namespace SOUND 
{
    //Chip-8 only produces one tone, it's frequency is up to us
    const int beep_tone = 440;
    auto beep_start = [&](){};
    auto beep_stop = [&](){};
    //Multiplatform beep function
    U0 make_beep(){beep_start();}
    U0 init_sound(){}
}

int main(int argc, char** argv)
{
    if(argc < 2) {std::cout << "No file specified, closing." << std::endl; return 0;}

    for(int i=0;i<argc;i++){
        if(std::string(argv[i]).find("-sq") != std::string::npos) SYSTEM::shift_quirks = true;
        if(std::string(argv[i]).find("-lsq") != std::string::npos) SYSTEM::load_store_quirks = true;
        if(std::string(argv[i]).find("-w") != std::string::npos) SYSTEM::screen_wraping = true;
        if(std::string(argv[i]).find("-d") != std::string::npos) SYSTEM::debug_execution = true;
    }

    U8* ROM = SYSTEM::load_file(argv[1]);
    if(ROM == nullptr) return -1;

    //Initialize everything
    CPU::init_cpu();
    MEMORY::init_memory(ROM);
    REGISTERS::init_registers();
    TIMERS::init_timers();
    DISPLAY::init_display();
    SOUND::init_sound();

    //Main loop
    for(;;)
    {
        IO::poll_events();
        if(((SDL_GetTicks() % (999/30))) == (999/30) - 1) {
            TIMERS::update_timers();
            CPU::opcode_t opcode = CPU::fetch_opcode();
            CPU::emulate_cycle(opcode);
        }
        SOUND::make_beep();
        DISPLAY::update_display();
    }
    return 0;
}