@echo off

pushd build

REM taskkill /IM remedybg.exe /F

cl ../src/main/c8_win.c /WX  /W4 /Z7  -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib  ole32.lib /Fe:c8_win.exe

REM cl ../src/test/test.c /Fe:c8_test.exe  /WX /W4 /Z7 -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib /link /SUBSYSTEM:CONSOLE

 cl ../src/assets/assets.c /Fe:c8_assets.exe  /WX /W4 /Z7 -Od -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib

 c8_assets.exe

REM c8_test.exe

del *.ilk *.obj

REM start remedybg c8_win.exe

popd