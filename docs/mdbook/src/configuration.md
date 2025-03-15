# Configuration

If you put ko6 in your home directory, you must source the `SourceMe.sh` file to set the environment. You can add `source $HOME/ko6/SourceMe.sh` in the `$HOME/.bashrc`. Once, the environment is set, in order to check if it works, run : `cd $HOME/ko6; make hello`, you must see two xterm windows appear. On the big one (xterm0), you must see, the logo and a system dump (internal registers and scheduler). On the small one, you must see « Hello World! ».

The source files are all 100 characters long because of the many comments on the lines. Therefore, 100-character wide windows must be used for editing. The C code uses the Kernighan and Ritchie style.