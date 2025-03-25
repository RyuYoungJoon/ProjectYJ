# Boost.Asio 네트워크 프로젝트

## 프로젝트 소개

Boost.Asio 라이브러리를 활용한 C++ 네트워크 통신 프레임워크입니다. 
비동기 서버-클라이언트 구조를 기반으로 하며 메모리 풀 및 객체 풀을 통한 메모리 관리 시스템을 구현하였습니다.

## 프로젝트 구조

ServerCore, GameServer, Client

### ServerCore

공통 기능을 제공하는 라이브러리 프로젝트입니다.

#### 네트워크 핵심 컴포넌트
- AsioSession: 네트워크 세션 관리
- AsioService: 네트워크 서비스 기능
- AsioIoContext: I/O 컨텍스트 관리
- AsioAcceptor: 클라이언트 연결 수락
- NetworkHandler: 네트워크 데이터 처리
- PacketRouter: 패킷 라우팅

#### 메모리 관리 컴포넌트
- ObjectPool: 객체 풀 관리
- PacketBuffer: 패킷 버퍼링
- SessionPacketBuffer: 송신 패킷 버퍼 관리

#### 유틸리티 컴포넌트
- TaskQueue: 작업 큐 관리
- ServerAnalyzer: 서버 성능 분석
- SessionManager: 세션 관리 및 추적
- MyLogger: 로깅 기능

### GameServer

게임 서버 프로젝트.

- GameServer.cpp: 서버 메인 진입점
- GameSession: 게임 세션 관리
- PacketHandler: 패킷 처리 로직

### Client

클라이언트 프로젝트.

- Client.cpp: 클라이언트 메인 진입점
- ClientSession: 클라이언트 세션 관리
- ClientManager: 클라이언트 연결 관리

## 데이터 흐름

1. 클라이언트가 서버에 연결 요청을 보냅니다.
2. AsioAcceptor가 연결을 수락하고 GameSession을 생성합니다.
3. 클라이언트가 서버로 패킷을 전송합니다.
4. 서버의 NetworkHandler가 패킷을 수신합니다.
5. PacketRouter가 패킷 유형에 따라 적절한 PacketHandler로 전달합니다.
6. PacketHandler가 패킷을 처리하고 응답을 생성합니다.
7. 서버가 응답 패킷을 클라이언트로 전송합니다.

## 설정 파일

서버와 클라이언트는 각각 `ServerConfig.ini`와 `ClientConfig.ini` 파일을 통해 구성됩니다.

### 서버 설정 예시

```ini
[server]
address=127.0.0.1
port=7777
ThreadCnt=4
PacketPoolSize = 10000
SessionPoolSize = 10000
```

### 클라이언트 설정 예시

```ini
[client]
Address=127.0.0.1
Port=7777
threadCnt=10
maxSessionCnt=100
PacketPoolSize = 10000
SessionPoolSize = 10000
```
