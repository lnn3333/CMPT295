#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/* Sign extends the given field to a 32-bit integer where field is
 * interpreted an n-bit integer. */
int sign_extend_number(unsigned int field, unsigned int n) {
  /* YOUR CODE HERE */
  return (int)field << (32 - n) >> (32 - n);
}

/* Unpacks the 32-bit machine code instruction given into the correct
 * type within the instruction struct */
Instruction parse_instruction(uint32_t instruction_bits) {
  /* YOUR CODE HERE */
  Instruction instruction;
  // add x8, x0, x0     hex : 00000433  binary = 0000 0000 0000 0000 0000 01000
  // Opcode: 0110011 (0x33) Get the Opcode by &ing 0x1111111, bottom 7 bits
  instruction.opcode = instruction_bits & ((1U << 7) - 1);

  // Shift right to move to pointer to interpret next fields in instruction.
  instruction_bits >>= 7;

  switch (instruction.opcode) {
  // R-Type
  case 0x33:
    // instruction: 0000 0000 0000 0000 0000 destination : 01000
    instruction.rtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 0000 0000 0000 0 func3 : 000
    instruction.rtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    // instruction: 0000 0000 0000  src1: 00000
    instruction.rtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // instruction: 0000 000        src2: 00000
    instruction.rtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    // funct7: 0000 000
    instruction.rtype.funct7 = instruction_bits & ((1U << 7) - 1);
    break;
  // case for I-type
  case 0x03:
  case 0x13:
  case 0x73:
    instruction.itype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.itype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.itype.imm = instruction_bits & ((1U << 12) - 1);
   
    break;
  // case for U-type
  case 0x37:
    instruction.utype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.utype.imm = instruction_bits & ((1U << 20) - 1);
   
    break;
  // case for J-type
  case 0x6f:
    instruction.ujtype.rd = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.ujtype.imm = instruction_bits & ((1U << 20) - 1);
   
    break;
  // case for S-type
  case 0x23:

    instruction.stype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.stype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.stype.imm7 = instruction_bits & ((1U << 7) - 1);
    break;
  // case for B-type
  case 0x63:
    instruction.sbtype.imm5 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.funct3 = instruction_bits & ((1U << 3) - 1);
    instruction_bits >>= 3;

    instruction.sbtype.rs1 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.rs2 = instruction_bits & ((1U << 5) - 1);
    instruction_bits >>= 5;

    instruction.sbtype.imm7 = instruction_bits & ((1U << 7) - 1);
    break;
  default:
    exit(EXIT_FAILURE);
  }
  return instruction;
}

/* Return the number of bytes (from the current PC) to the branch label using
 * the given branch instruction */

int get_branch_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  unsigned int imm_0 = 0;
  unsigned int imm_12 = (instruction.sbtype.imm7 >> 6) & 0x01;
  unsigned int imm_10_5 = (instruction.sbtype.imm7 ) & 0x3F;
  unsigned int imm_4_1 = (instruction.sbtype.imm5 >> 1) & 0x0F;
  unsigned int imm_11 = (instruction.sbtype.imm5 ) & 0x01;
  unsigned int imm_31_20 = (instruction.sbtype.imm7 >> 0) & 0x0F;

  // Sign-extend the immediate bits
  int imm = (imm_12 << 12) | (imm_11 << 11) | (imm_10_5 << 5) | (imm_4_1 << 1) | imm_0;
  int offset = sign_extend_number(imm, 13);  // Sign-extend to 13 bits

    // Calculate the branch offset

  return offset;
}

/* Returns the number of bytes (from the current PC) to the jump label using the
 * given jump instruction */

//ujtype
int get_jump_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  unsigned int imm_20 = (instruction.ujtype.imm >> 19) & 0x1;
  unsigned int imm_19_12 = (instruction.ujtype.imm) & 0xFF;
  unsigned int imm_11 = (instruction.ujtype.imm >> 8) & 0x1;
  unsigned int imm_10_1 = (instruction.ujtype.imm >> 9) & 0x3FF;

  // Calculate the jump offset
  int imm = (imm_20 << 20) | (imm_19_12 << 12) | (imm_11 << 11) | (imm_10_1 << 1) | 0;
  int offset = sign_extend_number(imm, 21);

  return offset;
}

//stype
int get_store_offset(Instruction instruction) {
  /* YOUR CODE HERE */
  unsigned int imm_4_0 = (instruction.stype.imm5); 
  unsigned int imm_11_5 = (instruction.stype.imm7); 
  int imm = (imm_11_5 << 5) | imm_4_0;
  int offset = sign_extend_number(imm, 11);
  return offset;
}

void handle_invalid_instruction(Instruction instruction) {
  printf("Invalid Instruction: 0x%08x\n", instruction.bits);
}

void handle_invalid_read(Address address) {
  printf("Bad Read. Address: 0x%08x\n", address);
  exit(-1);
}
                                                                                                                                                                                           
void handle_invalid_write(Address address) {
  printf("Bad Write. Address: 0x%08x\n", address);
  exit(-1);
}
