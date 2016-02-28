//
//  Chip-8.cpp
//  Chip-8
//
//  Created by Nolan Daigle on 2/26/16.
//  Copyright (c) 2016 Nolan Daigle. All rights reserved.
//
#include <iostream>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "Chip8.h"

unsigned char chip8_fontset[80] =
{
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
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void Chip8::Initialize()
{
    //Chip 8 expects that the application will be loaded at memory location 0x200
    pc = 0x200; //start program counter at 0x200
    //Opcode, index register, and stack pointer will all be initialized at 0
    opcode = 0;
    I = 0;
    sp = 0;
    
    //Set memory to 0
    for ( int i = 0; i < 4096; i++ )
        memory[i] = 0;
    
    //Set pixels to 0
    for ( int i = 0; i < 64*32; i++ )
        gfx[i] = 0;
    
    //Set registers and keys to 0
    for(int i = 0; i < 16; ++i)
        key[i] = V[i] = 0;
    
    //Clear stack
    for ( int i = 0; i < 16; ++i )
        stack[i] = 0;
    
    //Load Chip8 fontset
    for ( int i = 0; i < 80; ++i )
        memory[i] = chip8_fontset[i];
    
    //Set timers to 0
    delay_timer = 0;
    sound_timer = 0;
    
    //Set up SFML window stuff
    window.create(sf::VideoMode(800, 600), "Chip8 Emu" );
    
    running = true;
    
    drawFlag = true;
    
    srand(time(NULL));
}

void Chip8::LoadGame(std::string filename )
{
    // Buffer to store file into
    char buffer[3584];
    //Read file as binary
    std::ifstream file( filename.c_str(), std::ios::in | std::ios::binary );
    file.read(buffer, 3584);
    //Unload buffer into memory starting at 0x200, where the program starts in memory
    for ( int i = 0; i < 3584; ++i )
    {
        memory[i+512] = buffer[i];
        memory[i+512] = memory[i+512] & 0x00FF;
    }
    file.close();
    
    window.clear(sf::Color::Black);

}

void Chip8::EmulateCycle()
{
    unsigned short one = memory[pc];
    unsigned short two = memory[pc+1];
    unsigned short three = (one<<8) | two;
    
    //Fetch the opcode(which is 2 bytes) by merging the two program counter addresses
    opcode = three;
    
    //DECODE THE OPCODE
    switch(opcode & 0xF000)
    {
        case 0x0000:
            switch(opcode & 0x000F)
        {
            case 0x0000: // Clear the screen
                for(int i = 0; i < 2048; ++i)
                    gfx[i] = 0x0;
                drawFlag = true;
                pc += 2;
                break;
                
            case 0x000E: //Return from subroutine
                --sp;
                pc = stack[sp];
                pc += 2;
                break;
                
        }
            break;
            
        case 0x1000: // 0x1NNN: jump to address NNN=
            pc = opcode & 0x0FFF;
            break;
            
        case 0x2000: // 2NNN: Calls a subroutine at address NNN
            //Before calling a subroutine, save the current pc address in the stack
            stack[sp] = pc;
            ++sp;
            pc = opcode & 0x0FFF;
            break;
            
        case 0x3000: // 3XNN skips the next instruction if V[X] equals NN
            if ( V[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF) )
                pc += 2;
            pc += 2;;
            break;
            
        case 0x4000: // 4XNN skips the next instruction if V[X] DOES NOT equal NN
            if ( V[(opcode & 0x0F00) >> 8] != ( opcode & 0x00FF ) )
                pc += 2;
            pc += 2;
            break;
            
        case 0x5000: // 5XNN skips the next instruction if V[X] equals V[Y]
            if ( V[(opcode & 0x0F00) >> 8] == V[(opcode & 0x00F0) >> 4] )
                pc += 2;
            pc += 2;
            break;
            
        case 0x6000: // 0x6XNN sets V[X] to NN
            V[(opcode & 0x0F00) >> 8] = ( opcode & 0x00FF );
            pc += 2;
            break;
            
        case 0x7000: // 0x7XNN: Adds NN to V[X]
            V[(opcode & 0x0F00) >> 8] += ( opcode & 0x00FF );
            pc += 2;
            break;
            
        case 0x8000:
            
            switch(opcode & 0x000F)
        {
            case 0x0000: // 0x8XY0 Sets V[X] to the value of V[Y]
                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4];
                pc += 2;
                break;
                
            case 0x0001: // 0x8XY1 Sets V[X] to V[X] or V[Y]
                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] | V[(opcode & 0x00F0) >> 4];
                pc += 2;
                break;
                
            case 0x0002: // 0x8XY2 Sets V[X] to V[X] and V[Y]
                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                pc += 2;
                break;
                
            case 0x0003: // 0x8XY3 Sets V[X] to V[X] xor V[Y]
                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x0F00) >> 8] & V[(opcode & 0x00F0) >> 4];
                pc += 2;
                break;
                
            case 0x0004: // 0x8XY4: Add the value of V[X] to V[Y]. V[F], the carry flag, is set to 1 if the sum of V[X] and V[Y] is larger than 255 (each register cannot store numbers larger than 255)
                if(V[(opcode & 0x00F0) >> 4] > (0xFF - V[(opcode % 0x0F00) >> 8])) //
                    V[0xF] = 1; //carry flag set to 1
                else
                    V[0xF] = 0; //carry flag set to 0
                V[(opcode & 0x0F00) >> 8] += V[(opcode & 0x00F0) >> 4]; // Add the two numbers together
                pc += 2;
                break;
                
            case 0x0005: // 0x8XY5 V[Y] is subtracted from V[X]. VF is set to 0 when there's a borrow and 1 when there isn't.
                if(V[(opcode & 0x00F0) >> 4] > (V[(opcode % 0x0F00) >> 8])) //
                    V[0xF] = 0; //carry flag set to 0 if there is a borrow
                else
                    V[0xF] = 1; //carry flag set to 1 if there isn't
                V[(opcode & 0x0F00) >> 8] -= V[(opcode & 0x00F0) >> 4]; // Subtract the numbers
                pc += 2;
                break;
                
            case 0x0006: // 0x8XY6: Shift V[X] right by one. VF is set to value of the least significant bit of V[X] before the shift
                V[0xF] = ( V[ ( (opcode & 0x0F00 )>>8) ] & 0x1 ); // Save the last bit on the end that will by carried
                V[ ( (opcode & 0x0F00 ) >> 8 )]>>= 1;
                pc += 2;
                break;
                
            case 0x0007: // 0x8XY7: sets V[X] to V[Y] minus V[X]. V[F] is set to 0 when there's a borrow and 1 if there isn't
                if( (V[(opcode % 0x0F00) >> 8]) > V[(opcode & 0x00F0) >> 4] ) //
                    V[0xF] = 0; //carry flag set to 0 if there is a borrow
                else
                    V[0xF] = 1; //carry flag set to 1 if there isn't
                V[(opcode & 0x0F00) >> 8] = V[(opcode & 0x00F0) >> 4] - V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
                
            case 0x000E: // 0x8XYE: Shifts V[X] left by one. V[F] is set to the value of the most significant bit of V[X] before the shift
                V[0xF] = ( V[ ( (opcode & 0xF000 )>>12) ] & 0x1 ); // Save the bit at the end that will be carried.
                V[ ( (opcode & 0x0F00 ) >> 8 )]<<= 1;
                pc += 2;
                break;
                
        }
            
            break;
            
        case 0x9000: // 9XY0 Skips the next instruction if V[X] and V[Y] are not equal
            if ( V[(opcode & 0x0F00) >> 8] != V[(opcode & 0x00F0) >> 4] )
                pc += 2;
            pc += 2;
            break;
            
        case 0xA000: // ANNN: Set Index Register to the address NNN
            I = opcode & 0x0FFF;
            pc += 2;
            break;
            
        case 0xB000: // BNNN: Jumpts to address NNN plus V[0]
            stack[sp] = pc;
            ++sp;
            pc = (opcode & 0x0FFF) + V[0];
            break;
            
        case 0xC000: // CXNN: Sets V[X] to the result of bitwise & on a random number and NN
            V[(opcode & 0x0F00) >> 8] = ( rand() % 0xFF + 0x00 ) & (opcode & 0x00FF);
            pc += 2;
            break;
            
        case 0xD000:{ // Sprites stored in memory at location in index register (I), 8bits wide. Wraps around the screen. If when drawn, clears a pixel, register VF is set to 1 otherwise it is zero. All drawing is XOR drawing (i.e. it toggles the screen pixels). Sprites are drawn starting at position VX, VY. N is the number of 8bit rows that need to be drawn. If N is greater than 1, second line continues at position VX, VY+1, and so on.
            unsigned short x = V[(opcode & 0x0F00) >> 8]; // Get X value of sprite
            unsigned short y = V[(opcode & 0x00F0) >> 4]; // Get Y value of sprite
            unsigned short height = opcode & 0x000F; // Get height of sprite
            unsigned short pixel;
            
            V[0xF] = 0; // Reset carry flag to 0
            
            
            for ( int yline = 0; yline < height; yline++ ) // Loop through each row
            {
                pixel = memory[I+yline]; // Find pixel value of memory starting at location I
                for ( int xline = 0; xline < 8; xline++) // Loop over one 8bit row
                {
                    if ( (pixel & (0x80 >> xline ) ) != 0 ) // check if this current pixel value is set to 1
                    {
                        if ( gfx[ (x + xline + ( (y + yline)*64 ) ) ] == 1 ) // Check if pixel display is set to 1, if so, we set the V[F] carry flag to register a collision
                            V[0xF] = 1;
                        gfx[x + xline + ( (y+yline) * 64 ) ] ^= 1; //Set the pixel value
                    }
                }
            }
            
            drawFlag = true;  //GFX array has been changed, so we want to update the screen.
            pc += 2;
            
        }break;
            
        case 0xE000: //Input instructions
            switch(opcode & 0x000F)
        {
            case 0x000E: // 0xEX9E: Skips the next instruction if the key stored in VX is pressed.
                if (key[V[(opcode & 0x0F00) >> 8]] != 0)
                    pc += 2;
                pc += 2;
                break;
                
            case 0x0001: // Skips the next instruction if the key stored in VX isn't pressed.
                if (key[V[(opcode & 0x0F00) >> 8]] == 0)
                    pc += 2;
                pc += 2;
                break;
        }
            break;
            
        case 0xF000:
            switch(opcode & 0x000F)
        {
            case 0x0007: // 0xFX07: Sets VX to the value of the delay timer.
                V[(opcode & 0x0F00) >> 8] = delay_timer;
                pc += 2;
                break;
                
            case 0x000A: // 0xFX0A: A key press is awaited, and then stored in VX.
                 if (key[V[(opcode & 0x000F)]] == 0)
                pc += 2;
                break;
                
            case 0x0008: // 0xFX18: Sets the sound timer to VX.
                sound_timer = V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
                
            case 0x000E: // 0xFX1E: Adds VX to I.
                I += V[(opcode & 0x0F00) >> 8];
                pc += 2;
                break;
                
            case 0x0009: // 0xFX29: Sets I to the location of the sprite for the character in VX. Characters 0-F (in hexadecimal) are represented by a 4x5 font.
                pc += 2;
                break;
                
            case 0x0003: // 0xFX33: Stores the Binary-coded decimal representation of VX, with the most significant of three digits at the address in I, the middle digit at I plus 1, and the least significant digit at I plus 2. (In other words, take the decimal representation of VX, place the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.)
                memory[I] = V[(opcode & 0x0F00) >> 8] / 100;
                memory[I + 1] = (V[(opcode & 0x0F00) >> 8]/10) % 10;
                memory[I +2] = (V[(opcode & 0x0F00) >> 8] % 100) % 10;
                pc += 2;
                break;
                
            case 0x0005: // 0xFX55: A key press is awaited, and then stored in VX.
                switch(opcode & 0x00F0)
            {
                case 0x0010: // 0xFX15: Sets the delay timer to VX.
                    delay_timer = V[(opcode & 0x0F00) >> 8];
                    pc += 2;
                    break;
                    
                case 0x0050: // 0xFX55: Stores V0 to VX (including VX) in memory starting at address I.
                    for ( int i = 0; i < ((opcode & 0x0F00) >> 8); ++i )
                        memory[I+i] = V[i];
                    pc += 2;
                    break;
                    
                case 0x0060: // 0xFX65: Fills V0 to VX (including VX) with values from memory starting at address I.
                    for ( int i = 0; i < ((opcode & 0x0F00) >> 8); ++i )
                        V[i] = memory[I+i];
                    pc += 2;
                    break;
            }
                break;
        }
            break;
            
        default:
            std::cout<<"Unkown opcode: "<<opcode<<std::endl;
    }
    
    if( delay_timer > 0 )
        --delay_timer;
    if ( sound_timer > 0 )
    {
        if (sound_timer == 1 )
            std::cout<<"Buzzzzzzz"<<std::endl;
        --sound_timer;
    }
}

void Chip8::SetKeys()
{
    sf::Event event;
    while (window.pollEvent(event))
    {
        // Close window: exit
        if (event.type == sf::Event::Closed) {
            running = false;
            window.close();
        }
        
        for ( int i = 0; i < 0xF; ++i )
            key[i] = 0;
        
        // Escape pressed: exit
        if (event.type == sf::Event::KeyPressed )
        {
            switch ( event.key.code )
            {
                case sf::Keyboard::Escape:
                    running = false;
                    window.close();
                    break;
                case sf::Keyboard::Num1:
                    key[0] = 1;
                    break;
                case sf::Keyboard::Num2:
                    key[1] = 1;
                    break;
                case sf::Keyboard::Num3:
                    key[2] = 1;
                    break;
                case sf::Keyboard::Num4:
                    key[3] = 1;
                    break;
                case sf::Keyboard::Q:
                    key[4] = 1;
                    break;
                case sf::Keyboard::W:
                    key[5] = 1;
                    break;
                case sf::Keyboard::E:
                    key[6] = 1;
                    break;
                case sf::Keyboard::R:
                    key[7] = 1;
                    break;
                case sf::Keyboard::A:
                    key[8] = 1;
                    break;
                case sf::Keyboard::S:
                    key[9] = 1;
                    break;
                case sf::Keyboard::D:
                    key[10] = 1;
                    break;
                case sf::Keyboard::F:
                    key[11] = 1;
                    break;
                case sf::Keyboard::Z:
                    key[12] = 1;
                    break;
                case sf::Keyboard::X:
                    key[13] = 1;
                    break;
                case sf::Keyboard::C:
                    key[14] = 1;
                    break;
                case sf::Keyboard::V:
                    key[15] = 1;
                    break;
                    
            }
        }
    }
}

void Chip8::Draw()
{
    
    window.clear();
    for ( int row = 1; row < 32; ++row )
    {
        for ( int pixel = 0; pixel < 64; ++pixel )
        {
            if ( gfx[row*64 + pixel] == 1 )
            {
                sf::RectangleShape rect(sf::Vector2f( 12, 12 ) );
                rect.setPosition(pixel*12, row*12);
                window.draw( rect );
            }
        }
    }
    window.display();
    drawFlag = false;
}