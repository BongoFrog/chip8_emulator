#ifndef CHIP8_H
#define CHIP8_H
#include <stdint.h>
typedef struct {
  uint8_t memory[4096];
  uint8_t V[16];
  uint16_t I;
  uint16_t pc;

  uint16_t stack[16];
  uint8_t sp;

  uint8_t delay_timer;
  uint8_t sound_timer;

  uint8_t keypad[16];
  uint8_t display[64 * 32];

  uint16_t opcode;

} Chip8;

#endif
