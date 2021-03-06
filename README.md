# Linux Module - System CPU/RAM Usage

This code demonstrate how to create a linux module.

## Requierments:

Hardware required:
~~~
  Raspebbry pi 3,
  4 leds,
  1 Button.
~~~
Files Required:
~~~
  sys_status_module.c,
  readinfo.c,
  Makefile.
~~~
Link to the files from github: https://github.com/Kinneret-OSCourse/advanced-os-course-project-shragas

## Installation:

installtion instruction:
  1. Download the files from files Required (shown above)
  2. put the files in the same project.
  3. connect 4 leds to gpio pin 27, 5, 6, 13. you are free to use any color or order
  4. connect a button to to gpio pin 16  
  5. open terminal and navigate to the folder you downloaded the files (using cd command)
  6. type 'make'.

  example:
  ~~~
    saw@sawPC:~/Desktop/project$ ls

    output: Makefile  readinfo.c  README.md  sys_status_module.c

    saw@sawPC:~/Desktop/project$ make
~~~
  your terminal output should look like this:
~~~
    gcc -g -o readinfo readinfo.c
    make -C /lib/modules/4.10.0-26-generic/build M=/home/saw/Desktop/project modules
    make[1]: Entering directory '/usr/src/linux-headers-4.10.0-26-generic'
      CC [M]  /home/saw/Desktop/project/sys_status_module.o
      Building modules, stage 2.
      MODPOST 1 modules
      CC      /home/saw/Desktop/project/sys_status_module.mod.o
      LD [M]  /home/saw/Desktop/project/sys_status_module.ko
    make[1]: Leaving directory '/usr/src/linux-headers-4.10.0-26-generic'
~~~
  7. type in terminal "sudo insmod sys_status_module.ko" (without the quotes) and press enter.
  8. enter your super user password and press enter.
  9. now you should see the leds turn on, by default the led display cpu usage.
  10. to toggle the led to cpu/memory usage press the 'p' or 'P' (both work).
  11. to turn off all the leds without removing the module press on the button. press again the turn them on again.
  12. to remove the module you can type the command "make clean" from the files location or "sudo rmmod sys_status_module"

example:
~~~
     saw@sawPC:~/Desktop/project$ make clean

     terminal output should look like this:

     make -C /lib/modules/4.10.0-26-generic/build M=/home/saw/Desktop/project clean
     make[1]: Entering directory '/usr/src/linux-headers-4.10.0-26-generic'
      CLEAN   /home/saw/Desktop/project/.tmp_versions
      CLEAN   /home/saw/Desktop/project/Module.symvers
     make[1]: Leaving directory '/usr/src/linux-headers-4.10.0-26-generic'
     rm -f readinfo
     sudo rmmod sys_status_module
~~~

## License:

MIT License

Copyright (c) [2017] [Oron Swissa]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
