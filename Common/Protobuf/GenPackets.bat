pushd %~dp0
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --cpp_out=./ ./Struct.proto
protoc.exe -I=./ --cpp_out=./ ./Enum.proto

GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --request=Req --ack=Ack
GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --request=Ack --ack=Req

IF ERRORLEVEL 1 PAUSE

XCOPY /Y Enum.pb.h "../../GameServer"
XCOPY /Y Enum.pb.cc "../../GameServer"
XCOPY /Y Struct.pb.h "../../GameServer"
XCOPY /Y Struct.pb.cc "../../GameServer"
XCOPY /Y Protocol.pb.h "../../GameServer"
XCOPY /Y Protocol.pb.cc "../../GameServer"
XCOPY /Y ClientPacketHandler.h "../../GameServer"


XCOPY /Y Enum.pb.h "../../Client"
XCOPY /Y Enum.pb.cc "../../Client"
XCOPY /Y Struct.pb.h "../../Client"
XCOPY /Y Struct.pb.cc "../../Client"
XCOPY /Y Protocol.pb.h "../../Client"
XCOPY /Y Protocol.pb.cc "../../Client"
XCOPY /Y ServerPacketHandler.h "../../Client"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h

PAUSE