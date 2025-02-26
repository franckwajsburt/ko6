/*------------------------------------------------------------------------------------------------*\
   _     ___    __
  | |__ /'v'\  / /      \date       2025-02-26
  | / /(     )/ _ \     \copyright  2021 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT

  \file     commmon/ctype.c
  \author   Franck Wajsburt
  \brief    ascii service functions (table of types)

\*------------------------------------------------------------------------------------------------*/

uint32_t _digit[8] = {[0 ... 7]=0, [1]=0x03FF0000};                  // 0-9
uint32_t _upper[8] = {[0 ... 7]=0, [2]=0x07FFFFFE};                  // A-Z
uint32_t _lower[8] = {[0 ... 7]=0, [3]=0x07FFFFFE};                  // a-z 
uint32_t _space[8] = {[0 ... 7]=0, [0]=0x00003E00, [1]=0x00000001};  // \t \n \v \f \r ' ' 
uint32_t _punct[8] = {[0 ... 7]=0, [1]=0XFC00FFFE, [2]=0XF8000001, [3]=0X78000001}; // ! ... ~
uint32_t _xdigit[8]= {[0 ... 7]=0, [1]=0x03FF0000, [2]=0x0000007E, [3]=0x0000007E}; // 0-9 A-F a-f
