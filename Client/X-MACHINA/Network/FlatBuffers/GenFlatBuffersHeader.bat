@echo off
flatc --cpp Player.fbs
flatc --cpp Enum.fbs
flatc --cpp FBProtocol.fbs
flatc --cpp Transform.fbs
flatc --cpp Struct.fbs

GenFlatBufferPackets.exe --path=./FBProtocol.fbs --output=ServerFBsPktFactory --recv=SPkt_ --send=CPkt_

IF ERRORLEVEL 1 PAUSE

