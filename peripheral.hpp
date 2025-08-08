#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdint>

// Display colors
extern sf::Color bgColor;
extern sf::Color fgColor;

// Key state
extern bool keys[16];
extern bool keysReleased[16];

// Sound system
extern sf::SoundBuffer beepBuffer;
extern sf::Sound beepSound;
extern bool soundInitialized;

// Peripheral functions
int InitSound();
void drawDisplay(sf::RenderWindow& window);
int sfmlKeyMap(sf::Keyboard::Key key);
void updateTimers(float deltaTime);
void handleEvent(sf::Event& event);
