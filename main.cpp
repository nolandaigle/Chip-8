
//
// Disclamer:
// ----------
//
// This code will work only if you selected window, graphics and audio.
//
// Note that the "Run Script" build phase will copy the required frameworks
// or dylibs to your application bundle so you can execute it on any OS X
// computer.
//
// Your resource files (images, sounds, fonts, ...) are also copied to your
// application bundle. To get the path to these resource, use the helper
// method resourcePath() from ResourcePath.hpp
//

//SFML includes
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
//SFML helper to find resource path
#include "ResourcePath.hpp"
//My headers
#include "Chip8.h"

int main(int argc, char **argv)
{
    Chip8 chip8;
    chip8.Initialize();
    std::string file = resourcePath()+"Games/invaders.c8";
    chip8.LoadGame(file.c_str());
    
    while ( chip8.is_running() )
    {
        chip8.EmulateCycle();
        
        if ( chip8.drawFlag )
            chip8.Draw();
        
        chip8.SetKeys();
    }
    return EXIT_SUCCESS;
}
