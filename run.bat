@echo off
rem Compiles all the data structure files together and runs the program.
cd /d "%~dp0"
echo Compiling...
gcc main.c railway.c train_bst.c route_dll.c seat_array.c passenger_sll.c waiting_queue.c undo_stack.c coach_cll.c -o railway
if errorlevel 1 (
    echo.
    echo Compilation failed. If railway.exe is already open in another terminal, close it first.
    pause
    exit /b 1
)
echo Done. Starting the program...
echo.
railway.exe
