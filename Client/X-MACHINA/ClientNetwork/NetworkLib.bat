@echo off

rem 프로젝트 폴더 위치를 변수에 저장
set "SolutionPath=C:\MyGithub\Xmachina\Client\DirectX12_Project\Client"

rem 대상 폴더 경로 설정 (예시로 ClientNetwork 디렉토리 아래에 복사)
set "source=%SolutionPath%\X-NetworkLibrary\Executable"
set "destination=%SolutionPath%\X-MACHINA\ClientNetwork\Lib"

echo Copying files from %source% to %destination%...

xcopy /s /i /Y "%source%" "%destination%"

echo Copy completed.


@echo off

rem 소스 디렉토리와 대상 디렉토리 경로 설정
set "includesource=%SolutionPath%\X-NetworkLibrary\Network"
set "includedestination=%SolutionPath%\X-MACHINA\ClientNetwork\Include"

rem 소스 디렉토리의 .h 파일을 대상 디렉토리로 복사
xcopy /s /i /y "%includesource%\*.h" "%includedestination%"

echo Copy completed.
	