simple-vm
=========
a very simple virtual machine and assembler.

simple-vm is a 32-bit register based virtual machine. It shares similarities to a MIPS 
architecture because it has a total of 23 registers and all of the instructions are a 
fixed width of 32 bits.

usage
=====
```
$ git clone https://github.com/traplol/simple-vm.git
$ cd simple-vm
$ make
$ cd bin
$ ./assembler ../examples/recursive.asm -o recursive.bin
$ ./vm recursive.bin
120
```

Bytecode/binary layout
======================
| Offset | Size(bytes) | Purpose |
| ------ |:-----------:| ----------------:|
|  0x0               |      4                        | Size of data section |
|  0x5               |      4                        | Size of text section |
|  0x9               | Size of data section in bytes | The data section     |
|  0xD + SIZE_OF_DS  | Size of text section in bytes | Text section         |

Instruction layout
==================
```
Bit Key:
o - Opcode
r - register
m - immediate value
0 - ignored
```
All valid instructions follow one of the following bit layouts
### No operands
```
oooooo 00000000000000000000000000
  6               26
```
These types of instructions take no operands.

*Note: The 26 lowest bits are ignored.*

### Register to register
```
oooooo rrrrr rrrrr 0000000000000000
  6      5     5          16
```
These types of instructions take two operands in the form of register identifiers and 
will have the value of the second register applied to the value of the first register
with the result stored in the first register.

*Note: The 16 lowest bits are ignored.*

### Register to register with an offset
```
oooooo rrrrr rrrrr mmmmmmmmmmmmmmmm
  6      5     5          16
```
These types of instructions take three operands in the form of two register identifiers
and an immediate 16-bit value. The value of the second register and the immediate should
be added together and then the result applied to the first register storing the result
in the first register.

*Note: The sign bit of the 16-bit immediate is preserved*

### Immediate to register
```
oooooo rrrrr mmmmmmmmmmmmmmmmmmmmm
  6      5            21
```
These types of instructions take two operands in the form of one register identifer and
one immediate 21-bit value. The immediate value should be applied to the value of the
register and the result stored in the register.

*Note: The sign bit of the 21-bit immediate is preserved*

### Single register with no immediate:
```
oooooo rrrrr 000000000000000000000
  6      5            21
```
These types of instructions take a single operand in the form of a register identifier.

*Note: The 21 lowest bits are ignored.*

### Immediate with no register:
```
oooooo 00000 mmmmmmmmmmmmmmmmmmmmm
  6      5            21
```
These types of instructions take a single operand in the form of an immediate value stored
in the 21 lowest bits.

*Note: Bits 22-26, inclusive, are ignored.*

*Note: The sign bit of the 21-bit immediate is preserved*


