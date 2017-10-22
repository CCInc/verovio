:: PATH=D:\cygwin\bin;%PATH%
::D:\cygwin\bin\bash.exe -l D:/Projects/music/Verovio/verovio/tools/Emscripten/build.sh
::pause 

call D:\Data\em\emsdk_env.bat
cd D:\Projects\music\Verovio\verovio\emscripten
perl buildToolkit -DHPX
start D:\Projects\music\Verovio\verovio\tools\Emscripten\buildProd.bat
pause