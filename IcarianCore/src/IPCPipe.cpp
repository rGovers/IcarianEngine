// Icarian Engine - C# Game Engine
// 
// License at end of file.

#include "Core/IPCPipe.h"

#include <chrono>
#include <cstdio>

#ifndef WIN32
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"

namespace IcarianCore
{
    IPCPipe::IPCPipe()
    {
#ifdef WIN32
        m_pipeSock = INVALID_SOCKET;
#else
        m_pipeSock = -1;
#endif

        m_closed = false;
    }
    IPCPipe::~IPCPipe()
    {
#ifdef WIN32
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

    IPCPipe* IPCPipe::Accept(float a_timeoutSec) const
    {
        IERRBLOCK;

#ifdef WIN32
        const long timeoutSec = (long)a_timeoutSec;
        const long timeoutMicrosec = (long)((a_timeoutSec - timeoutSec) * 1000000.0);

        struct timeval timeout;
        timeout.tv_sec = timeoutSec;
        timeout.tv_usec = timeoutMicrosec;

        fd_set fdSet;
        FD_ZERO(&fdSet);
        FD_SET(m_pipeSock, &fdSet);

        IERRCHECKRET(select(m_pipeSock + 1, &fdSet, NULL, NULL, &timeout) != SOCKET_ERROR, nullptr);
        IERRCHECKRET(FD_ISSET(m_pipeSock, &fdSet), nullptr);

        const SOCKET pipeSock = accept(m_pipeSock, NULL, NULL);
        IERRCHECKRET(pipeSock != INVALID_SOCKET, nullptr);
        IERRDEFER(closesocket(pipeSock));

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = pipeSock;

        return pipe;
#else
        struct pollfd pollFd;
        pollFd.fd = m_pipeSock;
        pollFd.events = POLLIN;

        const int timeout = (int)(a_timeoutSec * 1000);

        IERRCHECKRET(poll(&pollFd, 1, timeout) >= 0, nullptr);
        IERRCHECKRET(pollFd.revents & POLLIN, nullptr);

        const int pipeSock = accept(m_pipeSock, NULL, NULL);
        IERRCHECKRET(pipeSock >= 0, nullptr);
        IERRDEFER(close(pipeSock));

        // This is not standard across UNIX/POSIX platforms so either need to set the socket option or use a send flag based on defines
#ifdef SO_NOSIGPIPE
        int setSigpipe = 1;
        setsockopt(pipeSock, SOL_SOCKET, SO_NOSIGPIPE, &setSigpipe, sizeof(setSigpipe));
#endif

#ifdef O_NONBLOCK
        const int flags = fcntl(pipeSock, F_GETFL, 0);
        IERRCHECKRET(flags >= 0, nullptr);
        IERRCHECKRET(fcntl(pipeSock, F_SETFL, flags | O_NONBLOCK) >= 0, nullptr);
#endif

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = pipeSock;

        return pipe;
#endif

        return nullptr;
    }

    IPCPipe* IPCPipe::Connect(const std::string_view& a_pipeName)
    {
        IERRBLOCK;

#ifdef WIN32
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
#else
        const int clientSock = socket(AF_UNIX, SOCK_STREAM, 0);
        IERRCHECKRET(clientSock >= 0, nullptr);
        IERRDEFER(close(clientSock));

#ifdef SO_NOSIGPIPE
        int setSigpipe = 1;
        setsockopt(clientSock, SOL_SOCKET, SO_NOSIGPIPE, &setSigpipe, sizeof(setSigpipe));
#endif

#ifdef O_NONBLOCK
        // Flags should be zero but get the flags regardless do not know what POSIX implementations will do
        const int flags = fcntl(clientSock, F_GETFL, 0);
        IERRCHECKRET(flags >= 0, nullptr);
        IERRCHECKRET(fcntl(clientSock, F_SETFL, flags | O_NONBLOCK) >= 0, nullptr);
#endif

        struct sockaddr_un serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sun_family = AF_UNIX;
        strncpy(serverAddr.sun_path, a_pipeName.data(), sizeof(serverAddr.sun_path) - 1);

        IERRCHECKRET(connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) >= 0, nullptr);
#endif

        IPCPipe* pipe = new IPCPipe();
        pipe->m_pipeSock = clientSock;

        return pipe;
    }
    IPCPipe* IPCPipe::Create(const std::string_view& a_pipeName)
    {
#ifdef WIN32
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

    bool IPCPipe::IsAlive() const
    {
        if (m_closed)
        {
            return false;
        }

#ifdef WIN32
        // TODO: Change this
        return true;
#else
        if (m_pipeSock < 0)
        {
            return false;
        }

        struct pollfd pfd = 
        {
            .fd = m_pipeSock,
            .events = POLLOUT,
        };

        if (poll(&pfd, 1, 1) < 0) 
        {
            return true;
        }

        return (pfd.revents & (POLLERR | POLLNVAL | POLLHUP)) == 0;
#endif
    }

#ifndef WIN32
    static CommunicationPipe::e_SendError SendData
    (
        int a_socket,
        const void* a_data,
        uint32_t a_size,
        const std::chrono::high_resolution_clock::time_point& startTime,
        int a_flags
    )
    {
        IERRBLOCK;

        uint32_t bytesSent = 0;
        while (bytesSent < a_size)
        {
            const int bytes = send(a_socket, a_data, a_size - bytesSent, a_flags);
            if (bytes > 0)
            {
                bytesSent += (uint32_t)bytes;

                continue;
            }

            // Capture the value incase someone else touches it
            const int err = errno;

            const std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
            IERRCHECKRET(std::chrono::duration<double>(now - startTime).count() < 5.0, CommunicationPipe::SendError_Timeout);

            if (bytes < 0)
            {
                IERRCHECKRET(err == EAGAIN || err == EWOULDBLOCK, CommunicationPipe::SendError_Fail);
            }
        }

        return CommunicationPipe::SendError_Success;
    }
#endif

    CommunicationPipe::e_SendError IPCPipe::Send(const PipeMessage& a_msg)
    {
        const std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

#ifdef WIN32
        IERRBLOCK;

        const int bytesSent = send(m_pipeSock, (const char*)&a_msg, PipeMessage::Size, 0);
        IERRCHECKRET(bytesSent >= 0, SendError_Fail);

        if (a_msg.Data != nullptr)
        {
            const int bytesSent = send(m_pipeSock, a_msg.Data, a_msg.Length, 0);
            IERRCHECKRET(bytesSent >= 0, SendError_Fail);
        }
#else
        constexpr int SendFlags = ILAMBDA(
        {
            int flags = 0;

#ifdef MSG_NOSIGNAL
            flags |= MSG_NOSIGNAL;
#endif

            ILRETURN flags;
        });

        const CommunicationPipe::e_SendError err = SendData(m_pipeSock, &a_msg, PipeMessage::Size, startTime, SendFlags);
        if (err != SendError_Success)
        {
            return err;
        }

        if (a_msg.Data != nullptr)
        {
            return SendData(m_pipeSock, a_msg.Data, a_msg.Length, startTime, SendFlags);
        }
#endif

        return SendError_Success;
    }
    bool IPCPipe::Receive(std::queue<PipeMessage>* a_messages)
    {
        IERRBLOCK;

#ifdef WIN32
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
                IERRCHECKRET(bytesReceived >= 0, false);

                IERRCHECKRET(msg.Type < PipeMessageType_End, false);
                IERRCHECKRET(bytesReceived == PipeMessage::Size, false);

                if (msg.Length > 0)
                {
                    msg.Data = new char[msg.Length];
                    IERRDEFER(delete[] msg.Data);

                    const int bytesReceived = recv(m_pipeSock, msg.Data, msg.Length, 0);
                    IERRCHECKRET(bytesReceived >= 0, false);
                }

                a_messages->push(msg);
            }
        }
#else
        struct pollfd pollFd =
        {
            .fd = m_pipeSock,
            .events = POLLIN
        };

#ifdef MSG_NOSIGNAL
        constexpr int ReceiveFlags = MSG_NOSIGNAL;
#else
        constexpr int ReceiveFlags = 0;
#endif

        while (poll(&pollFd, 1, 1) > 0)
        {
            if (pollFd.revents & (POLLNVAL | POLLERR))
            {
                ITRIGGERERRRET(false);
            }

            if (pollFd.revents & POLLHUP)
            {
                // So I found out that this event can come out of order from the rest of the events
                // Because of that we need to just set a flag and continue
                m_closed = true;

                while (true)
                {
                    PipeMessage msg = { };

                    const int bytesReceived = recv(m_pipeSock, &msg, PipeMessage::Size, ReceiveFlags);
                    if (bytesReceived <= 0)
                    {
                        break;
                    }

                    IERRCHECKRET(msg.Type < PipeMessageType_End, false);
                    IERRCHECKRET(bytesReceived == PipeMessage::Size, false);

                    if (msg.Length > 0)
                    {
                        msg.Data = new uint8_t[msg.Length];
                        IERRDEFER(delete[] msg.Data);

                        uint32_t bytesReceived = 0;
                        while (bytesReceived < msg.Length)
                        {
                            const int bytes = recv(m_pipeSock, msg.Data + bytesReceived, msg.Length - bytesReceived, ReceiveFlags);
                            IERRCHECKRET(bytes >= 0, false);

                            bytesReceived += (uint32_t)bytes;
                        }
                    }

                    a_messages->push(msg);
                }

                return true;
            }

            if (pollFd.revents & POLLIN)
            {
                PipeMessage msg = { };

                const int bytesReceived = recv(m_pipeSock, &msg, PipeMessage::Size, ReceiveFlags);
                IERRCHECKRET(bytesReceived >= 0, false);

                IERRCHECKRET(msg.Type < PipeMessageType_End, false);
                IERRCHECKRET(bytesReceived == PipeMessage::Size, false);

                if (msg.Length > 0)
                {
                    msg.Data = new uint8_t[msg.Length];
                    IERRDEFER(delete[] msg.Data);

                    uint32_t bytesReceived = 0;
                    while (bytesReceived < msg.Length)
                    {
                        const int bytes = recv(m_pipeSock, msg.Data + bytesReceived, msg.Length - bytesReceived, ReceiveFlags);
                        IERRCHECKRET(bytes >= 0, false);

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

// MIT License
// 
// Copyright (c) 2026 River Govers
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
