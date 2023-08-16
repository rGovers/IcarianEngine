#include "Flare/IPCPipe.h"
#include "Flare/IcarianDefer.h"

#include <cstdio>

#ifndef WIN32
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace FlareBase
{
    IPCPipe::IPCPipe()
    {
#if WIN32
        m_pipeSock = INVALID_SOCKET;
#else
        m_pipeSock = -1;
#endif
    }
    IPCPipe::~IPCPipe()
    {
#if WIN32
        if (m_pipeSock != INVALID_SOCKET)
        {
            closesocket(m_pipeSock);
        }
#else
        if (m_pipeSock >= 0)
        {
            close(m_pipeSock);
        }
#endif
    }

    IPCPipe* IPCPipe::Accept() const
    {
#if WIN32
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET(m_pipeSock, &fdSet);
        if (select(m_pipeSock + 1, &fdSet, NULL, NULL, &timeout) == SOCKET_ERROR)
        {
            perror("select");

            return nullptr;
        }

        if (!FD_ISSET(m_pipeSock, &fdSet))
        {
            return nullptr;
        }

        const SOCKET pipeSock = accept(m_pipeSock, NULL, NULL);
        if (pipeSock == INVALID_SOCKET)
        {
            perror("accept");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = pipeSock;
#else
        struct pollfd pollFd;
        pollFd.fd = m_pipeSock;
        pollFd.events = POLLIN;

        if (poll(&pollFd, 1, 5000) <= 0)
        {
            return nullptr;
        }

        if (!(pollFd.revents & POLLIN))
        {
            return nullptr;
        }

        const int pipeSock = accept(m_pipeSock, NULL, NULL);
        if (pipeSock < 0)
        {
            perror("accept");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = pipeSock;
#endif

        return pipe;
    }

    IPCPipe* IPCPipe::Connect(const std::string_view& a_pipeName)
    {
#if WIN32
        const SOCKET clientSock = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strncpy(serverAddr.sun_path, a_pipeName.data(), sizeof(serverAddr.sun_path) - 1);

        if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            perror("connect");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = clientSock;
#else
        const int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strncpy(serverAddr.sun_path, a_pipeName.data(), sizeof(serverAddr.sun_path) - 1);

        if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            perror("connect");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = clientSock;
#endif

        return pipe;
    }
    IPCPipe* IPCPipe::Create(const std::string_view& a_pipeName)
    {
#if WIN32
        // Failsafe to ensure the pipe is deleted
        DeleteFileA(a_pipeName.data());

        const SOCKET serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock == INVALID_SOCKET)
        {
            perror("socket");

            return nullptr;
        }

        struct sockaddr_un serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strncpy(serverAddr.sun_path, a_pipeName.data(), sizeof(serverAddr.sun_path) - 1);

        if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            perror("bind");

            return nullptr;
        }

        if (listen(serverSock, 1) == SOCKET_ERROR)
        {
            perror("listen");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = serverSock;
#else
        // Failsafe to ensure the pipe is deleted
        unlink(a_pipeName.data());

        const int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock < 0)
        {
            perror("socket");

            return nullptr;
        }

        struct sockaddr_un serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strncpy(serverAddr.sun_path, a_pipeName.data(), sizeof(serverAddr.sun_path) - 1);

        if (bind(serverSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0)
        {
            perror("bind");

            return nullptr;
        }

        if (listen(serverSock, 1) < 0)
        {
            perror("listen");

            return nullptr;
        }

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = serverSock;
#endif

        return pipe;
    }

    bool IPCPipe::Send(const PipeMessage& a_msg) const
    {
#if WIN32
        const int bytesSent = send(m_pipeSock, (const char*)&a_msg, PipeMessage::Size, 0);
        if (bytesSent < 0)
        {
            perror("send");

            return false;
        }
        
        if (a_msg.Data != nullptr)
        {
            const int bytesSent = send(m_pipeSock, a_msg.Data, a_msg.Length, 0);
            if (bytesSent < 0)
            {
                perror("send");

                return false;
            }
        }
#else
        // Theoretically could become an issue cause we are not checking is the whole message was sent
        // has not been an issue when running for long periods of time so ignoring as not running stuff that needs 100% uptime and lazy
        const int bytesSent = write(m_pipeSock, &a_msg, PipeMessage::Size);
        if (bytesSent < 0)
        {
            perror("write");

            return false;
        }

        if (a_msg.Data != nullptr)
        {
            uint32_t bytesSent = 0;

            while (bytesSent < a_msg.Length)
            {
                // Seems to get cutoff occasionally when sending large messages so we need to loop
                // seems to have fixed the occasional malformed message
                const int bytes = write(m_pipeSock, a_msg.Data + bytesSent, a_msg.Length - bytesSent);
                if (bytes < 0)
                {
                    perror("write");

                    return false;
                }

                bytesSent += (uint32_t)bytes;
            }
        }
#endif

        return true;
    }
    bool IPCPipe::Receive(std::queue<PipeMessage>* a_messages) const
    {
#if WIN32
        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 5;

        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET(m_pipeSock, &fdSet);
        while (select(m_pipeSock + 1, &fdSet, NULL, NULL, &timeout) > 0)
        {
            if (FD_ISSET(m_pipeSock, &fdSet))
            {
                PipeMessage msg;

                const int bytesReceived = recv(m_pipeSock, (char*)&msg, PipeMessage::Size, 0);
                if (bytesReceived < 0)
                {
                    perror("recv");

                    return false;
                }

                if (msg.Length > 0)
                {
                    msg.Data = new char[msg.Length];

                    const int bytesReceived = recv(m_pipeSock, msg.Data, msg.Length, 0);
                    if (bytesReceived < 0)
                    {
                        perror("recv");

                        return false;
                    }
                }

                a_messages->push(msg);
            }
        }
#else
        struct pollfd pollFd;
        pollFd.fd = m_pipeSock;
        pollFd.events = POLLIN;

        while (poll(&pollFd, 1, 1) > 0)
        {
            if (pollFd.revents & (POLLERR | POLLHUP | POLLNVAL))
            {
                return false;
            }

            if (pollFd.revents & POLLIN)
            {
                PipeMessage msg;

                const int bytesReceived = read(m_pipeSock, &msg, PipeMessage::Size);
                if (bytesReceived < 0)
                {
                    perror("read");

                    return false;
                }

                if (msg.Type >= PipeMessageType_End)
                {
                    return false;
                }

                if (bytesReceived != PipeMessage::Size)
                {
                    return false;
                }

                if (msg.Length > 0)
                {
                    msg.Data = new char[msg.Length];

                    uint32_t bytesReceived = 0;
                    while (bytesReceived < msg.Length)
                    {
                        const int bytes = read(m_pipeSock, msg.Data + bytesReceived, msg.Length - bytesReceived);
                        if (bytes < 0)
                        {
                            perror("read");

                            return false;
                        }

                        bytesReceived += (uint32_t)bytes;
                    }                    
                }

                a_messages->push(msg);
            }
        }
#endif

        return true;
    }
}
