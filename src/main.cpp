#include <SDL.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <windows.h>
#include <time.h>

#define W_WIDTH 1200
#define W_HEIGHT 600
#define S_WIDTH 64
#define S_HEIGHT 32
#define C_ON 0xc6baac
#define C_OFF 0x1e1c32
 
#define log(x) std::cout << x << std::endl

const int scodes[16] = {
    SDL_SCANCODE_X,
    SDL_SCANCODE_1,
    SDL_SCANCODE_2,
    SDL_SCANCODE_3,
    SDL_SCANCODE_Q,
    SDL_SCANCODE_W,
    SDL_SCANCODE_E,
    SDL_SCANCODE_A,
    SDL_SCANCODE_S,
    SDL_SCANCODE_D,
    SDL_SCANCODE_Z,
    SDL_SCANCODE_C,
    SDL_SCANCODE_4,
    SDL_SCANCODE_R,
    SDL_SCANCODE_F,
    SDL_SCANCODE_V};

struct gfx
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    SDL_Texture *texture = NULL;
};

bool drawFlag = false;

gfx init_gfx()
{
    gfx g;
    if((SDL_Init(SDL_INIT_EVERYTHING))!=0){
        fprintf(stderr, "SDL_Init failed, exiting...");
        exit(1);
    }
    if((g.window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, W_WIDTH, W_HEIGHT, 0)) == NULL){
        fprintf(stderr, "Failed to create window, exiting...");
        exit(1);
    }
    if((g.renderer = SDL_CreateRenderer(g.window, -1, 0)) == NULL){
        fprintf(stderr, "Failed to create renderer, exiting...");
        exit(1);
    }
    if((g.texture = SDL_CreateTexture(g.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, S_WIDTH, S_HEIGHT))==NULL){
        fprintf(stderr, "Failed to create texture, exiting...");
        exit(1);
    }

    return (g);
}

void draw_pixel(int x, int y, uint32_t *pixels)
{
    pixels[y * S_WIDTH + x] = (uint32_t)C_ON;
}

void clear_pixel(int x, int y, uint32_t *pixels)
{
    pixels[y * S_WIDTH + x] = (uint32_t)C_OFF;
}
uint32_t get_pixel(int x, int y, uint32_t *pixels)
{
    return pixels[y * S_WIDTH + x];
}

void clear_screen(uint32_t *pixels)
{
    for (int i = 0; i < S_HEIGHT * S_WIDTH; i++)
    {
        pixels[i] = (uint32_t)C_OFF;
    }
}

void draw_gfx(gfx *g, uint32_t *pixels)
{
    if(SDL_RenderClear(g->renderer) != 0){
        fprintf(stderr, "Render clear error, exiting...");
        exit(1);
    }
    if(SDL_UpdateTexture(g->texture, NULL, pixels, S_WIDTH * sizeof(uint32_t))!=0){
        fprintf(stderr, "Failed to update texture, exiting...");
        exit(1);
    }
    if(SDL_RenderCopy(g->renderer, g->texture, NULL, NULL)!=0){
        fprintf(stderr, "Render copy error, exiting...");
        exit(1);
    }
    SDL_RenderPresent(g->renderer);
    Sleep(1);
    
}

class CHIP8
{
public:
    uint16_t curr_opcode = 0;
    uint16_t pc = 0x200;
    uint16_t index = 0;
    uint16_t stack[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t sp = 0;
    uint8_t sound = 0;
    uint8_t delay = 0;
    uint8_t reg[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint8_t ram[4096];
    uint32_t display[S_HEIGHT * S_WIDTH];
    bool keydown = false;
    uint8_t keys[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    const uint8_t font[16 * 5] = {
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

    void fetch()
    {
        curr_opcode = ram[pc] << 8 | ram[pc + 1];
        pc += 2;
    }

    void decode()
    {
        uint8_t nibble[4];
        nibble[0] = (uint8_t)((curr_opcode & 0xF000) >> 12);
        nibble[1] = (uint8_t)((curr_opcode & 0x0F00) >> 8);
        nibble[2] = (uint8_t)((curr_opcode & 0x00F0) >> 4);
        nibble[3] = (uint8_t)((curr_opcode & 0x000F));

        switch (nibble[0])
        {
        case (0x0): // Clear Screen 00E0
            if (nibble[1] == 0 && nibble[2] == 0xE && nibble[3] == 0)
            {
                clear_screen(display);
            }
            else if (nibble[1] == 0 && nibble[2] == 0xE && nibble[3] == 0xE)
            {
                sp--;
                pc = stack[sp];
            }
            break;
        case (0x1): // Jump 1NNN
            pc = (nibble[1] << 8) | (nibble[2] << 4) | nibble[3];
            // std::cout<<"pc: "<<std::hex<<pc<<std::dec<<std::endl;
            break;
        case (0x2):
            stack[sp] = pc;
            sp++;
            pc = (nibble[1] << 8) | (nibble[2] << 4) | nibble[3];
            break;
        case (0x3):
            if (reg[nibble[1]] == ((nibble[2] << 4) | nibble[3]))
                pc += 2;
            break;
        case (0x4):
            if (reg[nibble[1]] != ((nibble[2] << 4) | nibble[3]))
                pc += 2;
            break;
        case (0x5):
            if (reg[nibble[1]] == reg[nibble[2]])
                pc += 2;
            break;
        case (0x6): // Set VX 6XNN
            reg[nibble[1]] = (nibble[2] << 4) | nibble[3];
            break;
        case (0x7): // Add NN to reg VX 7XNN
            reg[nibble[1]] += ((nibble[2] << 4) | nibble[3]);
            break;
        case (0x8):
        {
            switch (nibble[3])
            {
            case (0x0):
                reg[nibble[1]] = reg[nibble[2]];
                break;
            case (0x1):
                reg[nibble[1]] |= reg[nibble[2]];
                break;
            case (0x2):
                reg[nibble[1]] &= reg[nibble[2]];
                break;
            case (0x3):
                reg[nibble[1]] ^= reg[nibble[2]];
                break;
            case (0x4):
                if ((reg[nibble[1]] += reg[nibble[2]]) > 255)
                    reg[0xF] = 1;
                else
                    reg[0xF] = 0;
                break;
            case (0x5):
                reg[nibble[1]] -= reg[nibble[2]];
                break;
            case (0x6):
                reg[0xF] = (reg[nibble[1]] & 0x1);
                reg[nibble[1]] >>= 1;
                break;
            case (0x7):
                reg[nibble[1]] = reg[nibble[2]] - reg[nibble[1]];
                break;
            case (0xE):
                reg[0xF] = (reg[nibble[1]] >> 7);
                reg[nibble[1]] <<= 1;
                break;
            }
        }

        case (0x9):
            if (reg[nibble[1]] != reg[nibble[2]])
                pc += 2;
            break;
        case (0xA): // Set index to NNN ANNN
            index = (nibble[1] << 8) | (nibble[2] << 4) | nibble[3];

            break;
        case (0xB):
            pc = ((nibble[1] << 8) | (nibble[2] << 4) | nibble[3]) + reg[0];
            break;
        case (0xC):
        {
            int rgen = rand();
            reg[nibble[1]] = rgen & ((nibble[2] << 4) | nibble[3]);
            break;
        }
        case (0xD):
        { // Draw N rows from mem loc starting at I, starting at X,Y,  DXYN
            reg[0xF] = 0;
            uint8_t x = reg[nibble[1]] % 64;
            uint8_t y = reg[nibble[2]] % 32;
            uint8_t row;
            uint8_t pixel;
            for (int n = 0; n < nibble[3]; n++)
            {
                row = ram[index + n];
                for (int c = 0; c < 8; c++)
                {
                    pixel = (row & (0x80 >> c));
                    if (pixel != 0)
                    {
                        if (get_pixel(x + c, y + n, display) == C_ON)
                        {
                            // log("clearing at coords: " << x + c << ", " << y + n);
                            clear_pixel(x + c, y + n, display);
                        }
                        else
                        {
                            // log("drawing at coords: " << x + c << ", " << y + n);
                            draw_pixel(x + c, y + n, display);
                        }
                        drawFlag = true;
                    }
                }
            }
            break;
        }
        case (0xE):
        {
            switch ((nibble[2] << 4) | nibble[3])
            {
            case (0x9E):
                if (keys[reg[nibble[1]]] == 1)
                    pc += 2;
                log(std::hex << unsigned(reg[nibble[1]]) << std::dec);
                break;
            case (0xA1):
                if (keys[reg[nibble[1]]] == 0)
                    pc += 2;
                log(std::hex << unsigned(reg[nibble[1]]) << std::dec);
                break;
            }
        }
        case (0xF):
        {
            switch ((nibble[2] << 4) | nibble[3])
            {
            case (0x07):
                reg[nibble[1]] = delay;
                break;
            case (0x15):
                delay = reg[nibble[1]];
                break;
            case (0x18):
                sound = reg[nibble[1]];
                break;
            case (0x1E):
                index += reg[nibble[1]];
                if (index >= 0x1000)
                    reg[0xF] = 1;
                break;
            case (0x0A):
            {
                bool keyflag = false;
                for (int i = 0x0; i < 0xF; i++)
                {
                    if (keys[i] == 1)
                    {
                        reg[nibble[1]] = i;
                        keyflag = true;
                        break;
                    }
                }
                if (!keyflag)
                    pc -= 2;
                break;
            }
            case (0x55):
            {
                for (int r = 0; r <= nibble[1]; r++)
                {
                    ram[index + r] = reg[r];
                }
                break;
            }
            case (0x65):
            {
                for (int r = 0; r <= nibble[1]; r++)
                {
                    reg[r] = ram[index + r];
                }
                break;
            }
            }
            break;
        }
        }
    }
};

int main(int argc, char **argv)
{
    srand(time(NULL));
    time_t newtime = time(NULL) * 1000;
    time_t prevtime = time(NULL) * 1000;
    gfx g = init_gfx();
    CHIP8 cpu;
    memset(cpu.ram, 0, 4096);
    FILE *fd = fopen("chip8-test-suite.ch8", "rb");
    // std::cout << fd << std::endl;
    int p = 0x200;

    while (!feof(fd)) // to read file
    {
        // function used to read the contents of file
        fread(cpu.ram + 0x200, sizeof(cpu.ram) - 0x200, 1, fd);
    }

    fclose(fd);

    clear_screen(cpu.display);
    cpu.ram[0x1FF] = 0x5;
    cpu.ram[0x1FE] = 0x3;
    // std::cout << std::hex << unsigned(cpu.ram[0x3b3]) << std::dec << std::endl;

    bool isRunning = true;
    SDL_Event event;
    bool show_another_window = false;
    while (isRunning)
    {
        while (SDL_PollEvent(&event))
        {   

            switch (event.type)
            {
            case SDL_QUIT:
                isRunning = false;
                break;
            case SDL_KEYDOWN:

                for (int i = 0; i < 16; i++)
                {
                    if (event.key.keysym.scancode == scodes[i])
                    {
                        cpu.keys[i] = 1;
                    }
                    // log("Key: "<<std::hex<<i<<std::dec<<" is: "<<unsigned(cpu.keys[i]));
                }

                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
                {
                    isRunning = false;
                }
                break;
            case SDL_KEYUP:
                for (int i = 0; i < 16; i++)
                {
                    if (event.key.keysym.scancode == scodes[i])
                    {
                        cpu.keys[i] = 0;
                    }
                    // log("Key: "<<std::hex<<i<<std::dec<<" is: "<<unsigned(cpu.keys[i]));
                }
                break;
            }
        }

        cpu.fetch();
        // std::cout << "Current opcode: " << std::hex << cpu.curr_opcode << std::dec << std::endl;
        cpu.decode();
        if (drawFlag)
        {
            draw_gfx(&g, cpu.display);
            drawFlag = false;
        }
        newtime = time(NULL) * 1000;
        if (newtime - prevtime >= 16)
        {
            if (cpu.delay > 0)
                cpu.delay--;
            if (cpu.sound > 0)
                cpu.sound--;
        }
        prevtime = newtime;
    }


    SDL_DestroyRenderer(g.renderer);
    SDL_DestroyWindow(g.window);
    SDL_DestroyTexture(g.texture);
    SDL_Quit();

    return 0;
}
