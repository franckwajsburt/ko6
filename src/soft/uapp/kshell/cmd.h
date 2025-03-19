#ifndef CMD_H
#define CMD_H

enum cmd_type {
    WHILE,
    IF,
    BUILT_IN,
};

struct command {
    enum cmd_type t;
    char * argv[];
};

void if_command()

#endif