#include "chip8.h"
#define ROM_CAPACITY 3584
bool load_rom(Chip8 *chip, char *rom) {
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
    return false;
  }
  fseek(fp, 0, SEEK_SET);
  fread(chip->memory + 512, sizeof(uint8_t), romSize, fp);
  fclose(fp);
  return true;
}
