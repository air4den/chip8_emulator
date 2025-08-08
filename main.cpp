#include "interpreter.hpp"
#include "peripheral.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cstring>

// CPU timing
float instrPerSecond = 700.f;
float cycleInterval = 1.f / instrPerSecond;
float cpuAccumulator = 0.f;
sf::Clock cpu_clk;

int main(int argc, char* argv[]) {
    const char* romPath = "./roms/tests/ibm_logo.ch8";
    
    // Parse command line args
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--legacy") == 0 || strcmp(argv[i], "-l") == 0) {
            legacy = true;
            std::cout << "Using legacy COSMAC VIP behavior" << std::endl;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            std::cout << "Usage: " << argv[0] << " [options] [rom_path]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  --legacy, -l    Use original COSMAC VIP behavior" << std::endl;
            std::cout << "  --help, -h      Show this help message" << std::endl;
            std::cout << "Default ROM path: " << romPath << std::endl;
            return 0;
        } else if (argv[i][0] != '-') {
            romPath = argv[i];
        }
    }
    
    if (LoadROM(romPath) == -1 || LoadFont() < 0 || InitSound() < 0) {
        std::cout << "Failed init. Terminating program." << std::endl;
        return -1;
    }
    
    std::cout << "Starting CHIP-8 Emulator" << std::endl;
    std::cout << "ROM: " << romPath << std::endl;
    std::cout << "CPU Speed: " << instrPerSecond << " instructions per second" << std::endl;

    sf::RenderWindow window(sf::VideoMode(64*SZ_PIXEL, 32*SZ_PIXEL), "Chip8 Emulator");
    window.setFramerateLimit(60);

    cpu_clk.restart();

    while(window.isOpen()) {
        for (int i = 0; i < 16; i++) {
            keysReleased[i] = false;
        }
        
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) 
                window.close();
            handleEvent(event);
        }

        // CPU timing
        float cpu_dt = cpu_clk.restart().asSeconds();
        cpuAccumulator += cpu_dt;
        
        while (cpuAccumulator >= cycleInterval) {
            uint16_t instr = fetch();
            decodeExecute(instr);
            cpuAccumulator -= cycleInterval;
        }
        
        updateTimers(cpu_dt);

        // Render
        window.clear(bgColor);
        drawDisplay(window);
        window.display();
    }

    return 0;
}