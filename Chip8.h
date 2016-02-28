//
//  Chip-8.h
//  Chip-8
//
//  Created by Nolan Daigle on 2/26/16.
//  Copyright (c) 2016 Nolan Daigle. All rights reserved.
//

#ifndef __Chip_8__Chip_8__
#define __Chip_8__Chip_8__

class Chip8
{
private:
    //Current opcode
    unsigned short opcode;
    //Chip 8 has 4k in memory
    unsigned short memory[4096];
    //Chip 8 has 15 8-bit registers, the 16th register is used for the carry flag
    unsigned char V[16];
    //Index register
    unsigned short I;
    //Program counter
    unsigned short pc;
    
    //An array to hold each pixel on the screen
    unsigned char gfx[64*32];
    
    //The Chip 8 has two timer registers. If they are above 0 they will count down at 60hz. If the sound_timer reaches 0, the Chip 8 system will buzz
    unsigned char delay_timer;
    unsigned char sound_timer;
    
    //A stack to keep track of the current location before performing a jump or calling a subroutine.
    unsigned short stack[16];
    unsigned short sp;
    
    //Store the current state of the key (Chip 8 has a hex based keypad)
    unsigned char key[16];
    
    sf::RenderWindow window;
    
    bool running;
public:
    //Initialize the system
    void Initialize();
    
    //Load the Chip8 file
    void LoadGame(std::string filename );
    
    //Run through one cycle
    void EmulateCycle();
    
    //Draw to screen
    void Draw();
    
    //Sore key presses
    void SetKeys();
    
    bool drawFlag;
    
    bool is_running() { return running; }
    
};

#endif /* defined(__Chip_8__Chip_8__) */
