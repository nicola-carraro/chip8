@echo off

set common_flags=/WX /W4 /Z7  -nologo -Od

cl chip8.c %common_flags%     

cl test.c %common_flags%   /link  /SUBSYSTEM:CONSOLE

cl assets.c %common_flags%  

assets.exe

test.exe

del *.ilk *.obj