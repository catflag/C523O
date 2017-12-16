/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * <Dongpyeong Seo 20160759>
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
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

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

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

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
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
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
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
,
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
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */


/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  /* using the fact from the law of De Morgan which says that x AND y = NOT (NOT x OR NOT y)*/
  return ~(~x|~y);
}


/* 
 * leastBitPos - return a mask that marks the position of the
 *               least significant 1 bit. If x == 0, return 0
 *   Example: leastBitPos(96) = 0x20
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2 
 */
int leastBitPos(int x) {
  /* 
   * we can find that each bits of ~x+1 and x are all different EXCEPT the least 1 bit of x.
   * Therefore we can get what we want from x AND (NOT x + 1)
   */
  return x&(~x+1);
}



/* 
 * replaceByte(x,n,c) - Replace byte n in x with c
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: replaceByte(0x12345678,1,0xab) = 0x1234ab78
 *   You can assume 0 <= n <= 3 and 0 <= c <= 255
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 3
 */
int replaceByte(int x, int n, int c) {
  int shift_num = n << 3;
  int masking = ~(0xFF << shift_num); // ~ has more priority than <<.
  return (x & masking)|(c << shift_num);
}



/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /* the MSBs of 0 and 0's two's complement are both 0,
   * the MSBs of -2^31 and its two's complement are both 1, 
   * and the other cases they are (0,1), (1,0) since the sign of the original number and its two's complement are opposite. 
   * then, it is enough to check the MSBs of x and x's two's complement(~x+1) */
  return ((x|(~x+1))>>31)+1;
}
/*
 * leftBitCount - returns count of number of consective 1's in
 *     left-hand (most significant) end of word.
 *   Examples: leftBitCount(-1) = 32, leftBitCount(0xFFF0F0F0) = 12
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 50
 *   Rating: 4
 */
int leftBitCount(int x) {
  
  int minus1 = ~1 + 1;
  int minus2 = minus1 + minus1;
  int minus4 = minus2 + minus2;
  int minus8 = minus4 + minus4;
  int minus16 = minus8 + minus8;
  /*
  int mask16 = (1 << 16) + minus1; 
  int mask8 = 0xFF;
  int mask4 = 0xF;
  int mask2 = 3;
  int mask1 = 1; // 8
  */
  x = ~x;
  // Using binary search, we'll find the consecutive 1 from leading 1 of each interval(size 16 -> 8 -> 4 -> 2 -> 1 + LSB_exception)
  int cursor = 16; // each time we will check whether the left (32 - cursor) bits are all 1 or not.
  //int upper16_con = !(~(x >> cursor) & mask16); // 1 if consecutive and otherwise 0.
  int upper16_con = !(x >> cursor);
  cursor = cursor + 8 + (~(upper16_con + minus1) & minus16);

  //int upper8_con = !(~(x >> cursor) & mask8);
  int upper8_con = !(x >> cursor);
  cursor = cursor + 4 + (~(upper8_con + minus1) & minus8);

  //int upper4_con = !(~(x >> cursor) & mask4);
  int upper4_con = !(x >> cursor);
  cursor = cursor + 2 + (~(upper4_con + minus1) & minus4);

  //int upper2_con = !(~(x >> cursor) & mask2);
  int upper2_con = !(x >> cursor);
  cursor = cursor + 1 + (~(upper2_con + minus1) & minus2);

  //int upper1_con = !(~(x >> cursor) & mask1);
  int upper1_con = !(x >> cursor);
  // Exceptionally, this search does not work when x is -1 since it does not check LSB.
  // So I will check it separately. If x is -1, LSB_exception will be 1 and it will be added to the result.
  int LSB_exception = !x;

  return (upper16_con << 4) + (upper8_con << 3) + (upper4_con << 2) + (upper2_con << 1) + upper1_con + LSB_exception;
}
/* 
 * TMax - return maximum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmax(void) {
  /* max = 2^31 - 1 */
  return ~(1<<31);
}
/* 
 * implication - return x -> y in propositional logic - 0 for false, 1
 * for true
 *   Example: implication(1,1) = 1
 *            implication(1,0) = 0
 *   Legal ops: ! ~ ^ |
 *   Max ops: 5
 *   Rating: 2
 */
int implication(int x, int y) {
  /* x -> y <=> (NOT x) OR y */
  return !x|y;
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  /* two's complement */
  return ~x+1;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /* make a switch which become 0(0000....0(2)) or -1(1111....1(2)) so that we can turn only one of y or z on */
  int rev_switch = ~!x+1; // x != 0 -> rev_switch = 0, x = 0 -> rev_switch = -1 (11111.....1(2))
  return (~rev_switch&y)|(rev_switch&z); // precedence : !,~ > & > ^ > |
}
/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y){ 
  int x_MSB = x >> 31 & 1;
  int y_MSB = y >> 31 & 1;
  /* MSB
   * (1,0) / (0,1) -> overflow never occurs
   * (1,1) -> if MSB = 0 -> overflow / otherwise no overflow
   * (0,0) -> if MSB = 1 -> overflow / otherwise no overflow
   */
  return (x_MSB ^ y_MSB) | !(x_MSB ^ ((x + y) >> 31) & 1);
}
/* 
 * isGreater - if x > y  then return 1, else return 0 
 *   Example: isGreater(4,5) = 0, isGreater(5,4) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isGreater(int x, int y) {
  int x_MSBs = x >> 31; // 000000...0 or 111111...1 / in this case, it can save 1 more operator than using x_MSB = x >> 31 & 1
  int y_MSBs = y >> 31;
  /* 
   * (+) : nonnegative, (-) : negative
   * x : (+), y : (-) -> 1
   * x : (-), y : (+) -> 0
   * (+) - (+) = ?, (-) - (-) = ?, but no over/underflow will occur
   */
  return (!x_MSBs & y_MSBs) | !(x_MSBs ^ y_MSBs) & !(((y + (~x + 1)) >> 31) + 1);
}
/*
 * satMul3 - multiplies by 3, saturating to Tmin or Tmax if overflow
 *  Examples: satMul3(0x10000000) = 0x30000000
 *            satMul3(0x30000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0x70000000) = 0x7FFFFFFF (Saturate to TMax)
 *            satMul3(0xD0000000) = 0x80000000 (Saturate to TMin)
 *            satMul3(0xA0000000) = 0x80000000 (Saturate to TMin)
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 3
 */
int satMul3(int x) {
  int Tmin = 1 << 31;
  int Tmax = ~ (1 << 31);
  int x_MSBs = x >> 31;
  int x2 = x << 1;
  int x3 = x2 + x;
  int x2_MSBs = x2 >> 31;
  int x3_MSBs = x3 >> 31;
  /*
   * if one of x2_MSBs and x3_MSBs is different from x_MSBs, it means that overflow occurred.
   */
  int p_flag = ~x_MSBs & (x2_MSBs | x3_MSBs); // positive overflow flag - 0(flag off) or -1(flag on)
  int n_flag = x_MSBs & ~(x2_MSBs & x3_MSBs); // negative overflow flag - 0 or -1
  return p_flag & Tmax | n_flag & Tmin | ~(p_flag | n_flag) & x3;
} 
/* 
 * float_abs - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_abs(unsigned uf) {
  int exp_mask = 0x7f800000; //  0xFF << 23
  int flac_mask = 0x7fffff; // 2^23 - 1
  int exp = uf & exp_mask; // express only the exp bits. the other bits is 0.
  int max_exp_flag = !(exp ^ exp_mask); // if exp is 11111111(2), flag is on(1) and otherwise 0.
  int flac = uf & flac_mask;  // express only the flac bits. the other bits is 0.
  int NaN_flag = max_exp_flag && flac; // NaN_flag is 1 only if exp is 0xFF and flac is non zero.  if (NaN_flag)
  if (NaN_flag)
      return uf;
  return uf & 0x7fffffff; // MSB changes to 0 while preserving the other bits.
}
/* 
 * float_f2i - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
int float_f2i(unsigned uf) {
  int exp_mask = 0x7f800000; //  0xFF << 23
  int flac_mask = 0x7fffff; // 2^23 - 1
  int exp = uf & exp_mask; // express only the exp bits. the other bits is 0.
  int max_exp_flag = !(exp ^ exp_mask); // if exp is 11111111(2), flag is on(1) and otherwise 0.
  int min_exp_flag = !(exp & exp_mask); // if exp is 00000000(2), flag is on(1) and otherwise 0.
  int flac = uf & flac_mask;  // express only the flac bits. the other bits is 0.
  int bias = 127;
  int E = (exp>>23) - bias;
  if (max_exp_flag) // if exp = 111...11 <=> uf is infinite or NaN
	return 0x80000000;
  else if (min_exp_flag) // if exp = 000...00 <=> uf is denormalized
  {
	return 0;
  }
  else if (!((E - 31) >> 31)) // E >= 31 -> out of range, return 0x80000000 
  {
      return 0x80000000;
  }
  else if (E >> 31) // if E < 0 -> (int)uf always 0;
  {
      return 0;
  }
  else // shift the flac and omitted, leading 1 bit, and if the MSB of uf were 1, negate the result.
  {
      int temp = ((flac + (1 << 23)) >> (23 - E));
      if (uf >> 31)
          temp = ~temp + 1;
      return temp;
  }
}
