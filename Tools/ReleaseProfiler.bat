@echo off
rem Build Release with the CPU profiler enabled (/p:BeastProfiler=true) and run it.
rem Output: Game\x64\ReleaseProfiler\Game.exe  (normal Release output is not touched)
rem Usage: ReleaseProfiler.bat [run]   ... "run" skips the build and only launches the game
setlocal
set ROOT=%~dp0..
if /i "%~1"=="run" goto run

for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -prerelease -products * -find MSBuild\**\Bin\MSBuild.exe`) do set MSBUILD=%%i
if not defined MSBUILD (
  echo MSBuild.exe not found.
  exit /b 1
)
"%MSBUILD%" "%ROOT%\Game\Game.sln" -p:Configuration=Release -p:Platform=x64 -p:BeastProfiler=true -m -v:m -nologo
if errorlevel 1 (
  echo Build failed.
  exit /b 1
)

:run
rem Assets are loaded relative to the Game folder, so start the exe there
pushd "%ROOT%\Game"
start "" "%ROOT%\Game\x64\ReleaseProfiler\Game.exe"
popd
endlocal
