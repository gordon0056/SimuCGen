@echo off
REM Integration test runner for dsl-codegen (Windows)
setlocal enabledelayedexpansion

set BUILD_DIR=%~1
if "%BUILD_DIR%"=="" set BUILD_DIR=%~dp0..\build

set SCRIPT_DIR=%~dp0
set FIXTURES=%SCRIPT_DIR%fixtures
set DSL_CODEGEN=%BUILD_DIR%\Release\dsl-codegen.exe
set EXIT_CODE=0

echo === DSL Codegen Integration Tests ===

REM Test 1: PI Controller
echo.
echo Test 1: PI Controller generation
"%DSL_CODEGEN%" "%FIXTURES%\pi_controller.xml" > %TEMP%\pi_output.c 2>%TEMP%\pi_stderr.txt
if !ERRORLEVEL! EQU 0 (
    fc /b %TEMP%\pi_output.c "%FIXTURES%\pi_controller_expected.c" >nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        echo   PASS: Output matches expected
    ) else (
        echo   FAIL: Output differs from expected
        set EXIT_CODE=1
    )
) else (
    echo   FAIL: dsl-codegen exited with error
    type %TEMP%\pi_stderr.txt
    set EXIT_CODE=1
)

REM Test 2: Algebraic loop detection
echo.
echo Test 2: Algebraic loop detection
"%DSL_CODEGEN%" "%FIXTURES%\algebraic_loop.xml" >nul 2>%TEMP%\loop_stderr.txt
if !ERRORLEVEL! EQU 0 (
    echo   FAIL: Expected error exit code, but succeeded
    set EXIT_CODE=1
) else (
    findstr "algebraic loop detected" %TEMP%\loop_stderr.txt >nul
    if !ERRORLEVEL! EQU 0 (
        echo   PASS: Algebraic loop detected
    ) else (
        echo   FAIL: Wrong error message
        type %TEMP%\loop_stderr.txt
        set EXIT_CODE=1
    )
)

REM Test 3: Determinism check
echo.
echo Test 3: Determinism check
"%DSL_CODEGEN%" "%FIXTURES%\pi_controller.xml" > %TEMP%\pi_run1.c
"%DSL_CODEGEN%" "%FIXTURES%\pi_controller.xml" > %TEMP%\pi_run2.c
fc /b %TEMP%\pi_run1.c %TEMP%\pi_run2.c >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    echo   PASS: Outputs are byte-identical
) else (
    echo   FAIL: Outputs differ between runs
    set EXIT_CODE=1
)

echo.
if !EXIT_CODE! EQU 0 (
    echo All integration tests passed.
) else (
    echo Some tests failed.
)

exit /b !EXIT_CODE!
