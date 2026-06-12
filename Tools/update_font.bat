@echo off
cd /d "%~dp0.."

echo [charset.txt を生成中 (BMP全域 U+0001-U+FFFF)...]
powershell -ExecutionPolicy Bypass -Command "$cp=(1..65535)-join' ';[IO.File]::WriteAllText('Game\Assets\font\charset.txt',$cp,[Text.Encoding]::ASCII)"
if %ERRORLEVEL% neq 0 (
    echo [エラー] charset.txt の生成に失敗しました。
    pause
    exit /b 1
)

echo [フォントアトラス生成中... 少し時間がかかります]
Tools\msdf-atlas-gen.exe -font "Game/Assets/font/rounded-mplus-1c-heavy.ttf" -type sdf -format png -charset "Game/Assets/font/charset.txt" -imageout "Game/Assets/font/sdf_atlas.png" -json "Game/Assets/font/sdf_atlas.json" -size 48
if %ERRORLEVEL% neq 0 (
    echo [エラー] アトラス生成に失敗しました。
    pause
    exit /b 1
)

echo [完了] sdf_atlas.png / sdf_atlas.json を更新しました。
pause
