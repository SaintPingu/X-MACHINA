@echo off

rem ������Ʈ ���� ��ġ�� ������ ����
set "SolutionPath=C:\Users\MDH\Desktop\DirectX12_Project\Client"

rem ��� ���� ��� ���� (���÷� ClientNetwork ���丮 �Ʒ��� ����)``
set "source=%SolutionPath%\X-NetworkLibrary\Executable"
set "destination=%SolutionPath%\X-MACHINA\ClientNetwork\Lib"

echo Copying files from %source% to %destination%...

xcopy /s /i /Y "%source%" "%destination%"

echo Copy completed.


@echo off

rem �ҽ� ���丮�� ��� ���丮 ��� ����
set "includesource=%SolutionPath%\X-NetworkLibrary\Network"
set "includedestination=%SolutionPath%\X-MACHINA\ClientNetwork\Include"

rem �ҽ� ���丮�� .h ������ ��� ���丮�� ����
xcopy /s /i /y "%includesource%\*.h" "%includedestination%"

echo Copy completed.
	