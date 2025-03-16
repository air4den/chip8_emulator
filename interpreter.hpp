#include <cstdint>

class Interpreter {
    private:
        // 4KB memory
        std::uint8_t ram[4096];

        // 2^8 stack entries
        std::uint16_t stack[256];
        std::uint8_t sp;

        void push(std::uint16_t p);
        std::uint16_t pop();
    
    public:
        Interpreter();
        ~Interpreter();

        int load_rom();
        void run();

    };