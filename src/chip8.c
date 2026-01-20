#include "chip8.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define ROM_CAPACITY 3584
void chip8_init(chip8_t *chip) {
  memset(chip->memory, 0, 4096); // reset memory
  uint8_t fontSet[80] = {
      0xf0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xf0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xf0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xf0, 0x10, 0x10, // 4
      0xf0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xf0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xf0, 0x10, 0x20, 0x40, 0x40, // 7
      0xf0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xf0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xf0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xf0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xf0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xf0, 0x80, 0xF0, 0x80, 0x80  // F
  };
  for (int i = 0; i < 80; i++) {
    chip->memory[0x050 + i] = fontSet[i];
  }

  // reset register
  chip->pc = 0x200; // program start at 0x200
  chip->I = 0;
  memset(chip->V, 0, 16);
  chip->delay_timer = 0;
  chip->sound_timer = 0;
  chip->sp = 0;
  memset(chip->stack, 0, sizeof(chip->stack));
  memset(chip->display, 0, 64 * 32);
  chip->drawFlag = false;
  chip->waiting_for_input = false;
}

bool load_rom(chip8_t *chip, char *rom) {
  FILE *fp;
  fp = fopen(rom, "rb");
  if (!fp) {
    printf("Can't open the file\n");
    return false;
  }

  fseek(fp, 0, SEEK_END);
  long romSize = ftell(fp);
  if (romSize > ROM_CAPACITY) {
    printf("Rom exceed limit storage\n");
    fclose(fp);
    return false;
  }
  fseek(fp, 0, SEEK_SET);
  fread(chip->memory + 512, sizeof(uint8_t), romSize, fp);
  fclose(fp);
  return true;
}

uint16_t fetch(chip8_t *chip) {
  uint16_t opcode = chip->memory[chip->pc];
  opcode = (opcode << 8) | chip->memory[chip->pc + 1];
  return opcode;
}

void decode_and_execute(chip8_t *chip, uint16_t opcode) {
  bool advance_pc = true;
  uint8_t x = (opcode & 0x0F00) >> 8;
  uint8_t y = (opcode & 0x00F0) >> 4;
  uint8_t n = (opcode & 0x000F);
  uint8_t kk = (opcode & 0x00FF);
  uint16_t nnn = (opcode & 0x0FFF);

  switch (opcode & 0xf000) {
  case 0x0000:
    if (opcode == 0x00E0) {
      memset(chip->display, 0, sizeof(chip->display));
      chip->drawFlag = 1;
    } else if (opcode == 0x00EE) {
      chip->sp--;
      chip->pc = chip->stack[chip->sp];
      advance_pc = false;
    }
    break;
  case 0x1000:
    chip->pc = nnn;
    advance_pc = false;
    break;
  case 0x2000:
    chip->stack[chip->sp] = chip->pc;
    chip->sp++;
    chip->pc = nnn;
    advance_pc = false;
    break;
  case 0x3000:
    if (chip->V[x] == kk)
      chip->pc += 2;
    break;
  case 0x4000:
    if (chip->V[x] != kk)
      chip->pc += 2;
    break;
  case 0x5000:
    if (chip->V[x] == chip->V[y])
      chip->pc += 2;
    break;
  case 0x6000:
    chip->V[x] = kk;
    break;
  case 0x7000:
    chip->V[x] += kk;
    break;
  case 0x8000:
    switch (n) {
    case 0x0:
      chip->V[x] = chip->V[y];
      break;
    case 0x1:
      chip->V[x] |= chip->V[y];
      break;
    case 0x2:
      chip->V[x] &= chip->V[y];
      break;
    case 0x3:
      chip->V[x] ^= chip->V[y];
      break;
    case 0x4:
      chip->V[x] += chip->V[y];
      if (chip->V[x] >= chip->V[y])
        chip->V[0xf] = 0;
      else
        chip->V[0xf] = 1;
      break;
    case 0x5:
      if (chip->V[x] > chip->V[y])
        chip->V[0xf] = 1;
      else
        chip->V[0xf] = 0;
      chip->V[x] = chip->V[x] - chip->V[y];
      break;
    case 0x6:
      chip->V[0xf] = chip->V[x] & 0x0001;
      chip->V[x] = chip->V[x] >> 1;
      break;
    case 0x7:
      if (chip->V[y] > chip->V[x])
        chip->V[0xf] = 1;
      else
        chip->V[0xf] = 0;
      chip->V[x] = chip->V[y] - chip->V[x];
      break;
    case 0xe:
      chip->V[0xf] = chip->V[x] & 0x8000;
      chip->V[x] = chip->V[x] << 1;
      break;
    default:
      printf("Cant find opcode 8xyn\n");
    }
    break;
  case 0x9000:
    if (chip->V[x] != chip->V[y])
      chip->pc += 2;
    break;
  case 0xA000:
    chip->I = nnn;
    break;
  case 0xB000:
    chip->pc = nnn + chip->V[0];
    advance_pc = false;
    break;
  case 0xC000:
    uint8_t random = 0x67;
    chip->V[x] = kk & random;
    break;
  case 0xD000:
    chip->V[0xf] = 0;
    for (uint8_t row = 0; row < n; row++) {
      uint8_t spriteByte = chip->memory[chip->I + row];
      for (uint8_t col = 0; col < 8; col++) {
        uint8_t x_coordinate = (chip->V[x] + col) % 64;
        uint8_t y_coordinate = (chip->V[y] + row) % 32;
        uint8_t spriteBit = (spriteByte >> (7 - col)) & 0x01;
        if (spriteBit == 1) {
          chip->V[0xf] |= chip->display[x_coordinate + 64 * y_coordinate];
          chip->display[x_coordinate + 64 * y_coordinate] ^= spriteBit;
        }
      }
    }
    chip->drawFlag = 1;
    break;
  case 0xE000:
    if (kk == 0x9e) {

    } else if (kk == 0xa1) {
    }
    break;
  default:
    printf("Can't find the opcode or not implemented yet\n");
    break;
  }
  if (advance_pc)
    chip->pc += 2;
}
