/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date 2025-02-26
  | / /(     )/ _ \     Copyright (c) 2021 Sorbonne University
  |_\_\ x___x \___/     SPDX-License-Identifier: MIT

  \file     commmon/ctype.c
  \author   Franck Wajsburt
  \brief    ascii service functions (table of types)

\*------------------------------------------------------------------------------------------------*/

unsigned _digit[8] = {[0 ... 7]=0, [1]=0x03FF0000};                  // 0-9
unsigned _upper[8] = {[0 ... 7]=0, [2]=0x07FFFFFE};                  // A-Z
unsigned _lower[8] = {[0 ... 7]=0, [3]=0x07FFFFFE};                  // a-z 
unsigned _space[8] = {[0 ... 7]=0, [0]=0x00003E00, [1]=0x00000001};  // \t \n \v \f \r ' ' 
unsigned _punct[8] = {[0 ... 7]=0, [1]=0XFC00FFFE, [2]=0XF8000001, [3]=0X78000001}; // ! ... ~
unsigned _xdigit[8]= {[0 ... 7]=0, [1]=0x03FF0000, [2]=0x0000007E, [3]=0x0000007E}; // 0-9 A-F a-f
