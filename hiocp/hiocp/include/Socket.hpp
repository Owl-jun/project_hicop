#pragma once

namespace hiocp {

	class Socket
	{
	private:
        // ������ & �Ҹ���
        Socket(int domain, int type, int protocol);
        Socket(SOCKET sockfd); // Accept��
        ~Socket();

        // ����/�̵� ����
        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;
        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        // ���ε� & ����
        void bind(const std::string& ip, uint16_t port);
        void listen(int backlog = SOMAXCONN);
        Socket accept(); // ���ο� Ŭ���̾�Ʈ ���� ��ȯ
        void connect(const std::string& ip, uint16_t port);

        // ����
        void setReuseAddr(bool enable);
        void setNonBlocking(bool enable);

        // ��Ÿ
        SOCKET fd() const { return sockfd_; }
        bool isValid() const;

    private:
        SOCKET sockfd_;
        bool is_bound_ = false;
        bool is_connected_ = false;

        void close();  // ���� ������
        void throwLastError(const std::string& context);
	};

}

