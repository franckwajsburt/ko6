# kO6

```
 Small Operating System for Educational Purpose
  _     ___    __
 | |__ /'v'\  / /      \date        2022-07-13
 | / /(     )/ _ \     \copyright   2021-2022 Sorbonne University
 |_\_\ x___x \___/                  https://opensource.org/licenses/MIT
```

## kO6 license is MIT License

The chosen license is the most permissive I could find. In fact, the only requirement is to keep the banner at the beginning of the files. If you use kO6, let me know, I'll be happy to know and help you, if I can, to use it.

> Copyright © 2021-2022 Sorbonne University
>
> Permission is hereby granted, free of charge, to any person obtaining a copy of this software
> and associated documentation files (the "Software"), to deal with the Software without restriction,
> including without limitation the rights to use, copy, modify, merge, publish, distribute,
> sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
> furnished to do so, subject to the following conditions:
>
> The above copyright notice and this permission notice shall be included in all copies or
> substantial portions of the Software.
>
> The software is provided "as is", without warranty of any kind, express or implied, including
> but not limited to the warranties of merchantability, fitness for a particular purpose and
> noninfringement. In no event shall the authors or copyright holders be liable for any claim,
> damages or other liability, whether in an action of contract, tort or otherwise, arising from,
> out of or in connection with the software or the use or other dealings in the software.

## Author

> kO6 has been written by Franck Wajsburt - Sorbonne University
> contact : franck.wajsburt@lip6.fr

## Presentation

kO6 (*prononce it "kit-O-sys" O is  the letter not the number*) is a small operating system for educationnal purpose. Each part is added gradually, that is why it is a `k`it `O`perating `sys`tem (namely K-O-6).

## Configuration

If you put kO6 in your home directory, you must source the `SourceMe.sh` file to set the environment. You can add `source $HOME/kO6/SourceMe.sh` in the `$HOME/.bashrc`. Once, the environment is set, in order to check if it works, run : `cd $HOME/kO6; make hello`, you must see two xterm windows appear. On the big one (xterm0), you must see, the logo and a system dump (internal registers and scheduler). On the small one, you must see « Hello Word! ».

The source files are all 100 characters long because of the many comments on the lines. Therefore, 100-character wide windows must be used for editing. The C code uses the Kernighan and Ritchie style.
