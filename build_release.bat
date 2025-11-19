@echo off

if not exist build\release (
    meson setup build\release --vsenv -Dbuildtype=release
)

meson compile -C build\release

if not exist bin\release (
    mkdir bin\release
)

copy /Y build\release\subprojects\libepoxy-1.5.10\src\epoxy-0.dll bin\release\
copy /Y build\release\subprojects\glfw-3.4\glfw3-3.dll bin\release\
copy /Y build\release\subprojects\imgui-1.91.6\imgui.dll bin\release\
copy /Y build\release\voxels.exe bin\release\

bin\release\voxels.exe
