pushd build

cl ../src/main/c8_win.c /WX  /W4 /Z7  -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib /Fe:c8_win.exe

cl ../src/test/test.c /Fe:c8_test.exe  /WX /W4 /Z7 -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib /link /SUBSYSTEM:CONSOLE

cl ../src/assets/assets.c /Fe:c8_assets.exe  /WX /W4 /Z7 -Od -nologo -Od  kernel32.lib user32.lib D3d9.lib Dsound.lib

c8_assets.exe

c8_test.exe

del *.ilk *.obj

remedybg c8_win.exe

popd