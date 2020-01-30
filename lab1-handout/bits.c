/*
 * CS 208 Lab 1: Data Lab
 *
 * <Please put your name and userid here>
 *
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:

  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code
  must conform to the following style:

  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>

  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.


  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting if the shift amount
     is less than 0 or greater than 31.


EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implement floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants. You can use any arithmetic,
logical, or comparison operations on int or unsigned data.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to
     check the legality of your solutions.
  2. Each function has a maximum number of operations (integer, logical,
     or comparison) that you are allowed to use for your implementation
     of the function.  The max operator count is checked by dlc.
     Note that assignment ('=') is not counted; you may use as many of
     these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 *
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 10.0.0.  Version 10.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2017, fifth edition, plus
   the following additions from Amendment 1 to the fifth edition:
   - 56 emoji characters
   - 285 hentaigana
   - 3 additional Zanabazar Square characters */
/* We do not support C11 <threads.h>.  */
//2
/*
 * bitXor - x^y using only ~ and &
 *   Example: bitXor(4, 5) = 1
 *   Legal ops: ~ &
 *   Max ops: 14
 *   Rating: 2
 */
int bitXor(int x, int y) {
  int a = 0;
  int b = 0;
  a = (x & y);
  b = (~x&~y);
  return (~a & ~b);
}
/*
 * isTmax - returns 1 if x is the maximum, two's complement number,
 *     and 0 otherwise
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 2
 */
int isTmax(int x) {
  int y = x+1;
  x = x+y;
  y = !(y);
  x = ~x;
  x = x|y;
  x = !x;
  return x;
}
/*
 * allOddBits - return 1 if all odd-numbered bits in word set to 1
 *   where bits are numbered from 0 (least significant) to 31 (most significant)
 *   Examples allOddBits(0xFFFFFFFD) = 0, allOddBits(0xAAAAAAAA) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int allOddBits(int x) {
  int aBit = (((((0xAA << 8) | 0xAA) << 8) | 0xAA) << 8) | 0xAA;
  int deleteEvens = x & aBit;
  return !(aBit^deleteEvens);
}
/*
 * sign - return 1 if positive, 0 if zero, and -1 if negative
 *  Examples: sign(130) = 1
 *            sign(-23) = -1
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 10
 *  Rating: 3
 */
int sign(int x) {
    return (!!x) | (x >> 31);
}
//3
/*
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  int y = x + (~0x30 + 1);
  int sign_y = (y >> 31) & 1;
  int greater = !sign_y;
  // This checks if x is greater than 0x30

  int z = 0x39 + (~x + 1);
  int sign_z = (z >> 31) & 1;
  int less = !sign_z;
  // This checks if x is less than 0x39
  // Same logic for these checks as the isLessOrEqual function.
  return greater & less;
}
/*
 * isLessOrEqual - if x <= y  then return 1, else return 0
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  int signOfx = (x >> 31) & 1;
  int signOfy = (y >> 31) & 1;
  // Return 1 if x is negative and y is positive
  // Return 1 if y - x is positive
    // If both y and x are negative and y - x is positive then x is a greater negative number
    // If both y and x are negative and y -x is positive then y is a greater positive number

  int y_x = y + (~x + 1);
  int sign_yx = (y_x >> 31) & 1;
  // Checks the sign of subtracting the two. If sign_yx is 1 then y - x is negative.
  int result = (signOfx & !signOfy) | ((!(signOfx^signOfy)) & !sign_yx);
  // !(signOfx^signOfy) checks if they are the same sign
  // signOfx & !signOfy checks if they are not the same sign
  return result;
}
/*
 * logicalNeg - implement the ! operator, using all of
 *              the legal operators except !
 *   Examples: logicalNeg(3) = 0, logicalNeg(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int logicalNeg(int x) {
  return ((x | (~x + 1)) >> 31) + 1;
}
//4
/*
 * greatestBitPos - return a mask that marks the position of the
 *               most significant 1 bit. If x == 0, return 0
 *   Example: greatestBitPos(96) = 0x40
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 70
 *   Rating: 4
 */
int greatestBitPos(int x) {
  // We want to make all the adjacent bits equal to 1
  // Ex: Something like 0b 0010 0000 0000 0000 0000 0000 0000 0000
  // 0b 0011 0000 0000 0000 0000 0000 0000 0000
  // 0b 0011 1100 0000 0000 0000 0000 0000 0000
  // 0b 0011 1111 1100 0000 0000 0000 0000 0000
  // 0b 0011 1111 1111 1111 0000 0000 0000 0000
  // 0b 0011 1111 1111 1111 1111 1111 1111 1111

  // Invert
  // 0b 1100 0000 0000 0000 0000 0000 0000 0000
  // Right shift by 1
  // 0b 1110 0000 0000 0000 0000 0000 0000 0000
  // & with x makes the inverted right shift x leave the GSB.

  x = x | x >> 1;
  x = x | x >> 2;
  x = x | x >> 4;
  x = x | x >> 8;
  x = x | x >> 16;
  x = x & ((~x >> 1) ^ (1 << 31));
  // 1 << 31 xor takes care or negative cases.
  return x;
}
//float
/*
 * floatScale2 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 3
 */
unsigned floatScale2(unsigned uf) {
  // I wanted 0b0000000 01111111 11111111 11111111 = 0x007FFFFF
  int fracField = ((((0x7F << 8) | 0xFF) << 8) | 0xFF)
  //0
  if(uf == 0 || uf == (1 << 31))
    return uf;

  //Special value
  if(((uf >> 23) & 0xff) == 0xff)
  // This is when the exponent is just 0 so when it is denormalized.
    return uf;
  if(((uf >> 23) & 0xff) == 0x00)
    return ((uf & fracField) << 1) | ((1 << 31) & uf);
  // Normalized
  return uf + (1<<23);
}
/*
 * floatFloat2Int - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 3
 */
int floatFloat2Int(unsigned uf) {
  int sign = (uf >> 31) & 0x1;
  int e = (uf >> 23) & 0xFF;
  int frac = uf & 0x7FFFFF;

  int exponent = e - 127;
  // add the implicit one
  int newFrac = 0x1000000 + frac;
  int shifted;
  // if e equals zero -> denorm -> will be rounded to 0 while casting to integer
  // if exponent is negative -> will be rounded to 0 while casting to integer
  if(exponent < 0 || e == 0) {
    return 0;
  }
  // if exponent is greater than or equal to 31 -> overflow
  // if e == 0xFF -> special value
  if(exponent >= 31 || e == 0xFF) {
    return 0x80000000;
  }
  // if exponent is greater than 24, shift to left by (exponent - 24)
  if(exponent > 24) {
    shifted = newFrac << (exponent - 24);
  }
  // if exponent is less than or equal to 24, shift to right by (24 - exponent)
  else if(exponent <= 24) {
    shifted = newFrac >> (24 - exponent);
  }
  // negate if signed
  if(sign)
    shifted = -shifted;
  return shifted;
}
/*
 * floatPower2 - Return bit-level equivalent of the expression 2.0^x
 *   (2.0 raised to the power x) for any 32-bit integer x.
 *
 *   The unsigned value that is returned should have the identical bit
 *   representation as the single-precision floating-point number 2.0^x.
 *   If the result is too small to be represented as a denorm, return
 *   0. If too large, return +INF.
 *
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. Also if, while
 *   Max ops: 30
 *   Rating: 3
 */
unsigned floatPower2(int x) {
  if(x < -150) {
    return 0;
  }
  if(x >= -150 && x <= -127) {
    //denorm
    int shiftAmount = (-x - 127);
    int frac = 1 << shiftAmount;
    return frac;
  }
  if(x >= -126 && x <= 127) {
    //norm
    int e = (x + 127) << 23;
    return e;
  }
  if(x >= 128) {
    //inf
    int e = 0xFF << 23;
    return e;
  }
  return 0;
}
