# olc::PGE for Mac

This is a port of the [ConsoleGameEngine](https://github.com/OneLoneCoder/videos/blob/master/olcConsoleGameEngine.h), by [OneLoneCoder](https://onelonecoder.com/); The dependencies that you must have are:

  - Carbon (Installed with developer tools)

## Compiling

There is a makefile premade! Just type in the terminal:

```sh
$ cd olcPCEMac/
$ make
```

And there should be a file called 'ouput'.

## Differences and Possible Optimisations

The original Console Game Engine utilises the fact that the Windows Command Prompt's charachter buffer can be accessed directly with the <windows.h> header file. Of course, the same cannot be done on Mac. Instead of that, this implementation uses [escape sequences](http://www.termsys.demon.co.uk/vtansi.htm) that allow many different things. The most important here is the Force Cursor Position sequence, which allows to place the cursor back at the top left of the terminal.
