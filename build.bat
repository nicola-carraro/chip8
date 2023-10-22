@echo off

cl chip8.c /WX  /W4 /Z7  -nologo -Od       

cl test.c   /WX /W4 /Z7 -nologo -Od  /link  /SUBSYSTEM:CONSOLE

cl assets.c  /WX /W4 /Z7 -Od -nologo -Od 

assets.exe

test.exe

del *.ilk *.obj