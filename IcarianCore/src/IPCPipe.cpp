// Icarian Engine - C# Game Engine
//
// License at end of file.

#include "Core/IPCPipe.h"

#include <cstdio>

#ifndef WIN32
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

#include "Core/Bitfield.h"
#include "Core/IcarianError.h"
#include "Core/IcarianLambda.h"

namespace IcarianCore
{
#ifdef WIN32
    IPCPipe::IPCPipe(SOCKET a_socket)
    {
        ICARIAN_ASSERT(0);
    }
#else
    IPCPipe::IPCPipe(int a_socket)
    {
        m_pipeSock = a_socket;

        m_join = false;
        m_joined = false;

        m_flags = 0;

        m_sendOffset = 0;
        m_receiveOffset = 0;

        m_partialSend = { };
        m_partialReceive = { };

        m_thread = std::thread(Run, this);
    }
#endif
    IPCPipe::~IPCPipe()
    {
        m_join = true;

        while (!m_joined)
        {
            std::this_thread::yield();
        }

        m_thread.join();

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

        return new IPCPipe(pipeSock);
#else
        struct pollfd pollFd =
        {
            .fd = m_pipeSock,
            .events = POLLIN,
        };

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

        return new IPCPipe(pipeSock);
#endif

        return nullptr;
    }

    IPCPipe* IPCPipe::Connect(const char* a_pipeName)
    {
        IERRBLOCK;

#ifdef WIN32
        const SOCKET clientSock = socket(AF_UNIX, SOCK_STREAM, 0);

        struct sockaddr_un serverAddr = { };
        serverAddr.sun_family = AF_UNIX;
        for (uint32_t i = 0; i < sizeof(serverAddr.sun_path) - 1; ++i)
        {
            const char c = a_pipeName[i];

            serverAddr.sun_path[i] = c;

            if (c == 0)
            {
                break;
            }
        }

        if (connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
        {
            perror("connect");

            return nullptr;
        }

        return new IPCPipe(clientSock);
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

        struct sockaddr_un serverAddr = { };
        serverAddr.sun_family = AF_UNIX;
        for (uint32_t i = 0; i < sizeof(serverAddr.sun_path) - 1; ++i)
        {
            const char c = a_pipeName[i];

            serverAddr.sun_path[i] = c;

            if (c == 0)
            {
                break;
            }
        }

        IERRCHECKRET(connect(clientSock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) >= 0, nullptr);

        return new IPCPipe(clientSock);
#endif

        return nullptr;
    }
    IPCPipe* IPCPipe::Create(const char* a_pipeName)
    {
#ifdef WIN32
        // Failsafe to ensure the pipe is deleted
        DeleteFileA(a_pipeName);

        const SOCKET serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock == INVALID_SOCKET)
        {
            perror("socket");

            return nullptr;
        }

        struct sockaddr_un serverAddr = { };
        serverAddr.sun_family = AF_UNIX
        for (uint32_t i = 0; i < sizeof(serverAddr.sun_path) - 1; ++i)
        {
            const char c = a_pipeName[i];

            serverAddr.sun_path[i] = c;

            if (c == 0)
            {
                break;
            }
        }

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

        return new IPCPipe(serverSock);
#else
        // Failsafe to ensure the pipe is deleted
        unlink(a_pipeName);

        const int serverSock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (serverSock < 0)
        {
            perror("socket");

            return nullptr;
        }

        struct sockaddr_un serverAddr = { };
        serverAddr.sun_family = AF_UNIX;
        for (uint32_t i = 0; i < sizeof(serverAddr.sun_path) - 1; ++i)
        {
            const char c = a_pipeName[i];

            serverAddr.sun_path[i] = c;

            if (c == 0)
            {
                break;
            }
        }

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

        return new IPCPipe(serverSock);
#endif

        return nullptr;
    }

    bool IPCPipe::IsAlive() const
    {
#ifdef WIN32
        // TODO: Change this
        return true;
#else
        if (m_joined)
        {
            return false;
        }

        if (m_join)
        {
            return false;
        }

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

    void IPCPipe::Run(IPCPipe* a_pipe)
    {
        IERRBLOCK;

        IDEFER(a_pipe->m_joined = true);

        while (!a_pipe->m_join)
        {
#ifdef WIN32
            ICARIAN_ASSERT(0);
#else
            {
                IERRDEFER(printf("Failed to receive message \n"));

                struct pollfd pollFd =
                {
                    .fd = a_pipe->m_pipeSock,
                    .events = POLLIN
                };

                while (poll(&pollFd, 1, 1) > 0)
                {
                    if (pollFd.revents & (POLLNVAL | POLLERR))
                    {
                        return;
                    }

                    constexpr int ReceiveFlags = ILAMBDA(
                    {
                        int val = 0;

#ifdef MSG_NOSIGNAL
                        val |= MSG_NOSIGNAL;
#endif

                        ILRETURN val;
                    });

                    if (pollFd.revents & POLLHUP)
                    {
                        while (true)
                        {
                            PipeMessage msg = { };
                            uint32_t bytesReceived = 0;

                            if (IISBITSET(a_pipe->m_flags, PartialReadBit))
                            {
                                msg = a_pipe->m_partialReceive;
                                bytesReceived = a_pipe->m_receiveOffset;

                                ICLEARBIT(a_pipe->m_flags, PartialReadBit);
                            }
                            else
                            {
                                const int recvBytes = recv
                                (
                                    a_pipe->m_pipeSock,
                                    &msg,
                                    PipeMessage::Size,
                                    ReceiveFlags
                                );

                                if (recvBytes <= 0)
                                {
                                    break;
                                }

                                if (recvBytes != PipeMessage::Size)
                                {
                                    continue;
                                }
                                ICARIAN_ASSERT(msg.Type < PipeMessageType_End);

                                if (msg.Length > 0)
                                {
                                    msg.Data = new uint8_t[msg.Length];
                                }
                            }
                            IERRDEFER(
                            {
                                if (msg.Data != nullptr)
                                {
                                    delete[] msg.Data;
                                }
                            });

                            if (msg.Length > 0)
                            {
                                while (bytesReceived < msg.Length)
                                {
                                    const int bytes = recv(a_pipe->m_pipeSock, msg.Data + bytesReceived, msg.Length - bytesReceived, ReceiveFlags);
                                    IERRCHECK(bytes > 0);

                                    bytesReceived += bytes;
                                }

                                if (bytesReceived >= msg.Length)
                                {
                                    const std::unique_lock g = std::unique_lock(a_pipe->m_receiveLock);

                                    a_pipe->m_receiveQueue.emplace(msg);
                                }
                                else
                                {
                                    ISETBIT(a_pipe->m_flags, PartialReadBit);

                                    a_pipe->m_partialReceive = msg;
                                    a_pipe->m_receiveOffset = bytesReceived;
                                }
                            }
                            else
                            {
                                const std::unique_lock g = std::unique_lock(a_pipe->m_receiveLock);

                                a_pipe->m_receiveQueue.emplace(msg);
                            }
                        }

                        return;
                    }

                    if (pollFd.revents & POLLIN)
                    {
                        PipeMessage msg = { };
                        uint32_t bytesReceived = 0;

                        if (IISBITSET(a_pipe->m_flags, PartialReadBit))
                        {
                            msg = a_pipe->m_partialReceive;
                            bytesReceived = a_pipe->m_receiveOffset;

                            ICLEARBIT(a_pipe->m_flags, PartialReadBit);
                        }
                        else
                        {
                            const int recvBytes = recv(a_pipe->m_pipeSock, &msg, PipeMessage::Size, ReceiveFlags);
                            if (recvBytes != PipeMessage::Size)
                            {
                                continue;
                            }

                            ICARIAN_ASSERT(msg.Type < PipeMessageType_End);

                            if (msg.Length > 0)
                            {
                                msg.Data = new uint8_t[msg.Length];
                            }
                        }

                        IERRDEFER(
                        {
                            if (msg.Data != nullptr)
                            {
                                delete[] msg.Data;
                            }
                        });

                        if (msg.Length > 0)
                        {
                            while (bytesReceived < msg.Length)
                            {
                                const int bytes = recv(a_pipe->m_pipeSock, msg.Data + bytesReceived, msg.Length - bytesReceived, ReceiveFlags);
                                if (bytes < 0)
                                {
                                    const int err = errno;
                                    IERRCHECK(err == EAGAIN || err == EWOULDBLOCK);

                                    break;
                                }

                                if (bytes == 0)
                                {
                                    break;
                                }

                                bytesReceived += bytes;
                            }

                            if (bytesReceived >= msg.Length)
                            {
                                const std::unique_lock g = std::unique_lock(a_pipe->m_receiveLock);

                                a_pipe->m_receiveQueue.emplace(msg);
                            }
                            else
                            {
                                ISETBIT(a_pipe->m_flags, PartialReadBit);

                                a_pipe->m_partialReceive = msg;
                                a_pipe->m_receiveOffset = bytesReceived;
                            }
                        }
                        else
                        {
                            const std::unique_lock g = std::unique_lock(a_pipe->m_receiveLock);

                            a_pipe->m_receiveQueue.emplace(msg);
                        }
                    }
                }
            }

            {
                IERRDEFER(printf("Failed to send message \n"));

                constexpr int SendFlags = ILAMBDA(
                {
                    int flags = 0;

#ifdef MSG_NOSIGNAL
                    flags |= MSG_NOSIGNAL;
#endif

                    ILRETURN flags;
                });

                if (IISBITSET(a_pipe->m_flags, PartialWriteBit))
                {
                    while (a_pipe->m_sendOffset < a_pipe->m_partialSend.Length)
                    {
                        const int sent = send
                        (
                            a_pipe->m_pipeSock,
                            a_pipe->m_partialSend.Data + a_pipe->m_sendOffset,
                            a_pipe->m_partialSend.Length - a_pipe->m_sendOffset,
                            SendFlags
                        );

                        if (sent < 0)
                        {
                            const int err = errno;
                            IERRCHECK(err == EAGAIN || err == EWOULDBLOCK);

                            break;
                        }

                        if (sent == 0)
                        {
                            break;
                        }

                        a_pipe->m_sendOffset += sent;
                    }

                    if (a_pipe->m_sendOffset >= a_pipe->m_partialSend.Length)
                    {
                        delete[] a_pipe->m_partialSend.Data;
                        a_pipe->m_partialSend.Data = nullptr;

                        ICLEARBIT(a_pipe->m_flags, PartialWriteBit);
                    }
                }

                if (!IISBITSET(a_pipe->m_flags, PartialWriteBit))
                {
                    const std::unique_lock g = std::unique_lock(a_pipe->m_sendLock);

                    while (!a_pipe->m_sendQueue.empty())
                    {
                        PipeMessage& msg = a_pipe->m_sendQueue.front();

                        const int sent = send(a_pipe->m_pipeSock, &msg, PipeMessage::Size, SendFlags);
                        if (sent < 0)
                        {
                            const int err = errno;
                            IERRCHECK(err == EAGAIN || err == EWOULDBLOCK);

                            break;
                        }

                        if (sent != PipeMessage::Size)
                        {
                            break;
                        }

                        IDEFER(a_pipe->m_sendQueue.pop());
                        IERRDEFER(
                        {
                            if (msg.Data != nullptr)
                            {
                                delete[] msg.Data;
                                msg.Data = nullptr;
                            }
                        });

                        if (msg.Data != nullptr)
                        {
                            uint32_t bytesSent = 0;
                            while (bytesSent < msg.Length)
                            {
                                const int sent = send
                                (
                                    a_pipe->m_pipeSock,
                                    msg.Data + bytesSent,
                                    msg.Length - bytesSent,
                                    SendFlags
                                );

                                if (sent < 0)
                                {
                                    const int err = errno;
                                    IERRCHECK(err == EAGAIN || err == EWOULDBLOCK);

                                    break;
                                }

                                if (sent == 0)
                                {
                                    break;
                                }

                                bytesSent += sent;
                            }

                            if (bytesSent < msg.Length)
                            {
                                a_pipe->m_partialSend = msg;
                                a_pipe->m_sendOffset = bytesSent;

                                ISETBIT(a_pipe->m_flags, PartialWriteBit);
                            }
                            else
                            {
                                delete[] msg.Data;
                                msg.Data = nullptr;
                            }
                        }
                    }
                }
            }
#endif
        }
    }

    CommunicationPipe::e_SendError IPCPipe::Send(const PipeMessage& a_msg)
    {
        if (!IsAlive())
        {
            return SendError_Fail;
        }

        const bool validData = a_msg.Length > 0 && a_msg.Data != nullptr;
        const PipeMessage msg =
        {
            .Type = a_msg.Type,
            .Length = ILAMBDA(
            {
                if (validData)
                {
                    ILRETURN a_msg.Length;
                }

                ILRETURN uint32_t(0);
            }),
            .Data = ILAMBDA(
            {
                if (validData)
                {
                    uint8_t* val = new uint8_t[a_msg.Length];
                    memcpy(val, a_msg.Data, a_msg.Length);

                    ILRETURN val;
                }

                ILRETURN (uint8_t*)nullptr;
            }),
        };

        const std::unique_lock g = std::unique_lock(m_sendLock);

        m_sendQueue.emplace(msg);

        return SendError_Success;
    }
    bool IPCPipe::Receive(std::queue<PipeMessage>* a_messages)
    {
        ICARIAN_ASSERT(a_messages != nullptr);

        if (!IsAlive())
        {
            return false;
        }

        const std::unique_lock g = std::unique_lock(m_receiveLock);

        while (!m_receiveQueue.empty())
        {
            const PipeMessage& msg = m_receiveQueue.front();
            IDEFER(m_receiveQueue.pop());

            a_messages->emplace(msg);
        }

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
