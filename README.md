# project_hiocp
**WinSock 기반 IOCP 채팅서버**

---

## 프로젝트 개요

- WinSock 기반 IOCP 서버
- 비동기 Accept , 비동기 Send , Recv
- 멀티스레드 환경의 동시성 제어
- IOCP 구현에만 집중한 것이아닌, 서버 아키텍쳐 로서의 고민
- 서버 로직전용 큐 - 스레드풀 구조 도입
- I/O 작업 -> C API
- 서버로직 -> C++ RAII 기반으로 구현

---

## Directory

|폴더명|설명|
|:--:|:--:|
|구현중|구현중|

---

## API 문서 (ver 0.0.1)

> 구현중

---

## 향후 계획 (Roadmap)

- [x] IOCP 모델 구현 (WinSock) (250530)
- [ ] 가독성을 위한 헬퍼 클래스 구현
- [ ] I/O Task , ServerLogic Task 철저히 분리 (각각 전용 스레드풀)
- [ ] Session 등 원활한 서버 관리를 위한 클래스 설계
- [ ] 테스트
- [ ] 문서화

---

## 개발 일지

- 250530
    - [Velog포스팅](https://velog.io/@owljun/%EC%86%8C%EC%BC%93%ED%94%84%EB%A1%9C%EA%B7%B8%EB%9E%98%EB%B0%8D-WinSock-05.-%EB%B9%84%EB%8F%99%EA%B8%B0%EB%A1%9C-%ED%81%B4%EB%9D%BC%EC%9D%B4%EC%96%B8%ED%8A%B8-%EC%97%B0%EA%B2%B0%EC%9D%84-%EB%B0%9B%EC%9D%84%EB%95%8C-%EB%AC%B4%EC%8A%A8%EC%9D%BC%EC%9D%B4-%EC%9D%BC%EC%96%B4%EB%82%A0%EA%B9%8C-mswsock-AcceptEx)
    - IOCP 구조 , 비동기 Accept , Send , Recv 학습 후 처음 구현
    - 설계의 복잡성과 소켓과 컨텍스트 등 동적할당된 객체들의 생명주기 컨트롤에 대한 문제 인식
    - 처음 내린 결론 : 둘 다 한번에 해결할 수 있는 C++ RAII 개념을 들고와보자.
