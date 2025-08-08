#include "peripheral.hpp"
#include "interpreter.hpp"
#include <cmath>
#include <iostream>

// Display colors
sf::Color bgColor = sf::Color(20, 20, 40);
sf::Color fgColor = sf::Color(100, 200, 100);

// Key state
bool keys[16] = {};
bool keysReleased[16] = {};

// Sound system
sf::SoundBuffer beepBuffer;
sf::Sound beepSound;
bool soundInitialized = false;

// Timer constants
constexpr float TIMER_INTERVAL = 1.f / 60.f;

int InitSound() {
    const unsigned int sampleRate = 44100;
    const float frequency = 440.0f;
    const float duration = 0.1f;
    const unsigned int numSamples = static_cast<unsigned int>(sampleRate * duration);
    
    std::vector<sf::Int16> samples(numSamples);
    
    for (unsigned int i = 0; i < numSamples; ++i) {
        float t = static_cast<float>(i) / sampleRate;
        samples[i] = static_cast<sf::Int16>(32767.0f * std::sin(2.0f * M_PI * frequency * t));
    }
    
    if (!beepBuffer.loadFromSamples(samples.data(), numSamples, 1, sampleRate)) {
        std::cerr << "Error: Failed to create beep sound" << std::endl;
        return -1;
    }
    
    beepSound.setBuffer(beepBuffer);
    beepSound.setVolume(50.0f);
    soundInitialized = true;
    return 0;
}

void drawDisplay(sf::RenderWindow& window) {
    for (int y = 0; y < 32; ++y) {
        for (int x = 0; x < 64; ++x) {
            int idx = y * 64 + x;
            if (display[idx]) {
                sf::RectangleShape px(sf::Vector2f(SZ_PIXEL, SZ_PIXEL));
                px.setPosition(x * SZ_PIXEL, y * SZ_PIXEL);
                px.setFillColor(fgColor);
                window.draw(px);
            }
        }
    }
}

int sfmlKeyMap(sf::Keyboard::Key key) {
    switch (key) {
        case sf::Keyboard::Num1: return 0x1;
        case sf::Keyboard::Num2: return 0x2;
        case sf::Keyboard::Num3: return 0x3;
        case sf::Keyboard::Num4: return 0xC;
        case sf::Keyboard::Q:    return 0x4;
        case sf::Keyboard::W:    return 0x5;
        case sf::Keyboard::E:    return 0x6;
        case sf::Keyboard::R:    return 0xD;
        case sf::Keyboard::A:    return 0x7;
        case sf::Keyboard::S:    return 0x8;
        case sf::Keyboard::D:    return 0x9;
        case sf::Keyboard::F:    return 0xE;
        case sf::Keyboard::Z:    return 0xA;
        case sf::Keyboard::X:    return 0x0;
        case sf::Keyboard::C:    return 0xB;
        case sf::Keyboard::V:    return 0xF;
        default: return -1;
    }
}

void handleEvent(sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        int k = sfmlKeyMap(event.key.code);
        if (k != -1) {
            keys[k] = true;
        }
    }
    if (event.type == sf::Event::KeyReleased) {
        int k = sfmlKeyMap(event.key.code);
        if (k != -1) {
            keys[k] = false;
            keysReleased[k] = true;
        }
    }
}

void updateTimers(float deltaTime) {
    static float timerAccumulator = 0.f;
    timerAccumulator += deltaTime;
    
    while (timerAccumulator >= TIMER_INTERVAL) {
        if (delay_timer > 0)
            delay_timer -= 1;

        if (sound_timer > 0) {
            sound_timer -= 1;
            if (soundInitialized && beepSound.getStatus() != sf::Sound::Playing) {
                beepSound.play();
            }
        } else {
            if (soundInitialized && beepSound.getStatus() == sf::Sound::Playing) {
                beepSound.stop();
            }
        }
        timerAccumulator -= TIMER_INTERVAL;
    }
}