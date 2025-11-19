@echo off

if not exist build\debug (
    meson setup build\debug --vsenv
)

meson compile -C build\debug

if not exist bin\debug (
    mkdir bin\debug
)

copy /Y build\debug\subprojects\libepoxy-1.5.10\src\epoxy-0.dll bin\debug\
copy /Y build\debug\subprojects\glfw-3.4\glfw3-3.dll bin\debug\
copy /Y build\debug\subprojects\imgui-1.91.6\imgui.dll bin\debug\
copy /Y build\debug\voxels.exe bin\debug\

bin\debug\voxels.exe
