@echo off

if not exist build (
    meson setup build --vsenv
)

meson compile -C build

if not exist bin (
    mkdir bin
)

copy /Y build\subprojects\libepoxy-1.5.10\src\epoxy-0.dll bin\
copy /Y build\subprojects\glfw-3.4\glfw3-3.dll bin\
copy /Y build\voxels.exe bin\

bin\voxels.exe
