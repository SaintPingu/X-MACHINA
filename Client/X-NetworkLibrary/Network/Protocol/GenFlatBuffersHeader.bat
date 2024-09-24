@echo off
flatc --cpp Enum.fbs
flatc --cpp FBProtocol.fbs
flatc --cpp Transform.fbs
flatc --cpp Struct.fbs


flatc --cpp FBP_Bullet.fbs
flatc --cpp FBP_Chat.fbs
flatc --cpp FBP_Enemy.fbs
flatc --cpp FBP_Enter.fbs
flatc --cpp FBP_Item.fbs
flatc --cpp FBP_LogIn.fbs
flatc --cpp FBP_NetworkLatency.fbs
flatc --cpp FBP_Phero.fbs
flatc --cpp FBP_Player.fbs
flatc --cpp FBP_Custom.fbs

IF ERRORLEVEL 1 PAUSE
