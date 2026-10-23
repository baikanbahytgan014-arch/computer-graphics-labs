@echo off
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
    echo Не найден g++. Установи MinGW-w64 или MSYS2, затем добавь g++ в PATH.
    pause
    exit /b 1
)
g++ -std=c++17 -O2 -Wall -Wextra -static -mwindows main.cpp -lopengl32 -lgdi32 -luser32 -o pixel_patrol.exe
if errorlevel 1 (
    pause
    exit /b 1
)
start "" "%~dp0pixel_patrol.exe"
