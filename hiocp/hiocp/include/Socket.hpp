#pragma once

namespace hiocp {

	class Socket
	{
	private:
        // 생성자 & 소멸자
        Socket(int domain, int type, int protocol);
        Socket(SOCKET sockfd); // Accept용
        ~Socket();

        // 복사/이동 제어
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        // 바인딩 & 연결
        void bind(const std::string& ip, uint16_t port);
        void listen(int backlog = SOMAXCONN);
        Socket accept(); // 새로운 클라이언트 소켓 반환
        void connect(const std::string& ip, uint16_t port);

        // 설정
        void setReuseAddr(bool enable);
        void setNonBlocking(bool enable);

        // 기타
        SOCKET fd() const { return sockfd_; }
        bool isValid() const;

    private:
        SOCKET sockfd_;
        bool is_bound_ = false;
        bool is_connected_ = false;

        void close();  // 내부 정리용
        void throwLastError(const std::string& context);
	};

}

