@echo off
setlocal enabledelayedexpansion

REM 检查Git是否可用
git --version >nul 2>&1
if errorlevel 1 (
    echo Warning: Git not found, using default version info
    set GIT_BRANCH=unknown
    set GIT_TIME=unknown
    set GIT_COMMIT_HASH=unknown
    set GIT_COMMIT_SHORT=unknown
) else (
    for /f %%i in ('git rev-parse --abbrev-ref HEAD') do set GIT_BRANCH=%%i
    for /f %%i in ('git show --oneline --format^="%%ci%%H" -s HEAD') do set GIT_TIME=%%i
    for /f %%i in ('git rev-parse HEAD') do set GIT_COMMIT_HASH=%%i
    for /f %%i in ('git rev-parse --short HEAD') do set GIT_COMMIT_SHORT=%%i
)

REM 生成version.h文件
(
echo #pragma once
echo.
echo #define APP_VERSION "V2.1.0"
echo #define GIT_BRANCH "!GIT_BRANCH!"
echo #define GIT_TIME "!GIT_TIME!"
echo #define GIT_COMMIT_HASH "!GIT_COMMIT_HASH!"
echo #define GIT_COMMIT_SHORT "!GIT_COMMIT_SHORT!"
echo #define GIT_VERSION "Git: !GIT_BRANCH!: !GIT_TIME! (!GIT_COMMIT_HASH!)"
) > version.h

echo Generated version.h successfully