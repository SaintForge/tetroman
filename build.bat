@echo off

REM cl /nologo /EHsc -Zi win32_platform.cpp -DASSET_BUILD=1 -DDEBUG_BUILD=1 /I%SDL2LIB%"/include" /link shell32.lib /LIBPATH:%SDL2LIB%/lib/x64 SDL2main.lib SDL2.lib  SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib /SUBSYSTEM:WINDOWS

cl /nologo /EHsc -Zi win32_platform.cpp -DDEBUG_BUILD /I%SDL2LIB%"/include" /link /LIBPATH:%SDL2LIB%/lib/x64 SDL2main.lib SDL2.lib  SDL2_image.lib SDL2_ttf.lib SDL2_mixer.lib

mv *.obj build\
mv *.exe build\
mv *.pdb build\
mv *.ilk build\
