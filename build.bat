@echo off

cl chip8.c /WX  /W4 /Z7  -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib  ole32.lib Comdlg32.lib 

cl test.c   /WX /W4 /Z7 -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib /link ole32.lib Comdlg32.lib  /SUBSYSTEM:CONSOLE

cl assets.c  /WX /W4 /Z7 -Od -nologo -Od  kernel32.lib user32.lib D3d9.lib  Comdlg32.lib Dsound.lib 

assets.exe

test.exe

del *.ilk *.obj