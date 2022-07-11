/*------------------------------------------------------------------------------------------------*\
   _     ___    __  
  | |__ /'v'\  / /      \date       2022-07-02
  | / /(     )/ _ \     \copyright  2021-2022 Sorbonne University
  |_\_\ x___x \___/                 https://opensource.org/licenses/MIT 
  
  \file     commmon/esc_code.h
  \author   Franck Wajsburt
  \brief    ANSI escape code (https://www.wikiwand.com/en/ANSI_escape_code)
            - Add theses ascii sequences to get font-style and colors
            - For example : fprintf (EC_RED "Hello World %s\n" EC_RESET

\*------------------------------------------------------------------------------------------------*/


#ifndef _ESC_CODE_H_
#define _ESC_CODE_H_

#define EC_RESET           "\033[0m"
#define EC_BOLD            "\033[1m"
#define EC_FAINT           "\033[2m"
#define EC_ITALIC          "\033[3m"
#define EC_UNDERLINE       "\033[4m"

#define EC_BLACK           "\033[30m"
#define EC_RED             "\033[31m"
#define EC_GREEN           "\033[32m"
#define EC_YELLOW          "\033[33m"
#define EC_BLUE            "\033[34m"
#define EC_MAGENTA         "\033[35m"
#define EC_CYAN            "\033[36m"
#define EC_WHITE           "\033[37m"
#define EC_ORANGE          "\033[38;5;202m"

#define EC_BLACK_BG        "\033[40m"
#define EC_RED_BG          "\033[41m"
#define EC_GREEN_BG        "\033[42m"
#define EC_YELLOW_BG       "\033[43m"
#define EC_BLUE_BG         "\033[44m"
#define EC_MAGENTA_BG      "\033[45m"
#define EC_CYAN_BG         "\033[46m"
#define EC_WHITE_BG        "\033[47m"

#endif//_ESC_CODE_H_
