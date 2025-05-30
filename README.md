# project_hiocp
**WinSock 기반 IOCP 정적 라이브러리**

---

## 프로젝트 개요

- WinSock을 사용할 때의 불편한 C 스타일 API 사용을 개선.
- IOCP 기반 비동기 네트워크 프로그래밍을 객체지향적으로 추상화
- 메서드 기반의 직관적인 API 제공
- 실전 서버 개발에 활용 가능한 모듈형 정적 라이브러리

---

## Quick Start (ver 0.0.1)


```cpp
#include <hiocp.hpp>

int main() {
    hiocp::NetSystem netInit;
    // TODO: Create sockets, bind to IOCP, run GQCS loop
}
```

---

## Directory

|폴더명|설명|
|:--:|:--:|
|hiocp|전체 프로젝트 루트|
|/hiocp|정적 라이브러리 소스코드|
|/include|공개 헤더 파일 (.hpp)|
|/src|구현부 (.cpp)|
|/test|단위 테스트용|

---

## API 문서 (ver 0.0.1)

`namespace hiocp`
`class NetSystem`
|메서드|설명|
|:--:|:--:|
|NetSystem()|WSAStartup(ver 2.2)을 호출하여 WinSock 초기화|
|~NetSystem()|소멸 시 WSACleanup() 자동 호출 (RAII 기반)|
> NetSystem은 네트워크 기능 사용을 위해 가장 먼저 생성되어야 하며,
모든 네트워크 구성요소의 베이스 역할을 담당합니다.

---

## 향후 계획 (Roadmap)

- IOCP 채팅 서버 구현 (WinSock 활용)
- 리팩토링 하면서 라이브러리화

---

## 개발일지

[VELOG](https://velog.io/@owljun/series/hiocp%EC%9E%91%EC%97%85%EA%B8%B0)