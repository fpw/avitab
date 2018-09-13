/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2018 Folke Will <folko@solhost.org>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU Affero General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Affero General Public License for more details.
 *
 *   You should have received a copy of the GNU Affero General Public License
 *   along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <vector>
#include <stdexcept>
#define _POSIX_SOURCE
#include <unistd.h>
#include "AuthServer.h"

#ifdef WIN32
#include <windows.h>
#endif

namespace navigraph {

AuthServer::AuthServer() {
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void AuthServer::setAuthCallback(AuthCallback cb) {
    onAuth = cb;
}

void AuthServer::start(int port) {
    srvSock = socket(AF_INET, SOCK_STREAM, 0);
    if (srvSock < 0) {
        throw std::runtime_error("Couldn't create server socket");
    }

    int val = 1;
    setsockopt(srvSock, SOL_SOCKET, SO_REUSEADDR, (char *) &val, sizeof(val));

    sockaddr_in serverAddr {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddr.sin_port = htons(port);

    if (bind(srvSock, (sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
        close(srvSock);
        throw std::runtime_error("Couldn't bind server socket to port " + port);
    }

    if (listen(srvSock, 5) < 0) {
        close(srvSock);
        throw std::runtime_error("Couldn't listen on server socket");
    }

    keepAlive = true;
    serverThread = std::make_unique<std::thread>(&AuthServer::loop, this);
}

void AuthServer::loop() {
    while (keepAlive) {
        sockaddr_in clientAddr {};
        int clientLen = sizeof(clientAddr);

        fd_set readSet, masterSet;
        FD_ZERO(&masterSet);
        FD_ZERO(&readSet);

        FD_SET(srvSock, &masterSet);
        int fdMax = srvSock;
        readSet = masterSet;

        timeval timeout{};
        timeout.tv_usec = 1000 * 100;
        if (select(fdMax + 1, &readSet, nullptr, nullptr, &timeout) < 0) {
            break;
        }

        if (FD_ISSET(srvSock, &readSet)) {
            int clientSock = accept(srvSock, (sockaddr *) &clientAddr, &clientLen);
            handleClient(clientSock);
            shutdown(clientSock, SHUT_RDWR);
            close(clientSock);
        }
    }
    shutdown(srvSock, SHUT_RDWR);
    close(srvSock);
}

void AuthServer::handleClient(int client) {
    method.clear();
    url.clear();
    version.clear();
    curHeaderField.clear();
    curHeaderValue.clear();
    contentBytesLeft = 0;
    postFields.clear();
    curPostField.clear();
    curPostValue.clear();
    state = State::METHOD;

    do {
        char buf[255] = {0, };
        int readNow = recv(client, buf, sizeof(buf), 0);
        if (readNow <= 0) {
            break;
        }
        if (!handleData(buf)) {
            break;
        }
    } while (true);

    std::string reply;
    if (method == "POST") {
        try {
            if (onAuth) {
                onAuth(postFields);
                reply = "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\nAviTab: Success, you can close this tab now!";
            }
        } catch (const std::exception &e) {
            reply = std::string("HTTP/1.1 500 Internal Error\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\nAviTab Error: ") + e.what();
        }
    } else {
        reply = "HTTP/1.1 400 NOT FOUND\r\nConnection: close\r\nContent-Type: text/plain\r\n\r\nAviTab: 404 Not Found";
    }
    send(client, reply.c_str(), reply.length(), 0);
}

bool AuthServer::handleData(const std::string& data) {
    for (char c: data) {
        switch (state) {
        case State::METHOD:
            if (!isspace(c)) {
                method.push_back(c);
            } else {
                state = State::URL;
            }
            break;
        case State::URL:
            if (!isspace(c)) {
                url.push_back(c);
            } else {
                state = State::VERSION;
            }
            break;
        case State::VERSION:
            if (c != '\r' && c != '\n') {
                version.push_back(c);
            } else if (c == '\n') {
                curHeaderField.clear();
                state = State::FIELD;
            }
            break;
        case State::FIELD:
            if (c != ':' && c != '\r' && c != '\n') {
                curHeaderField.push_back(c);
            } else if (c == ':') {
                curHeaderValue.clear();
                state = State::VALUE;
            } else if (c == '\n') {
                // end of header
                if (contentBytesLeft > 0) {
                    state = State::POST_FIELD;
                } else {
                    return false;
                }
            }
            break;
        case State::VALUE:
            if (c != '\r' && c != '\n') {
                curHeaderValue.push_back(c);
            } else if (c == '\n') {
                if (curHeaderField == "Content-Length") {
                    contentBytesLeft = std::stoul(curHeaderValue);
                }
                curHeaderField.clear();
                state = State::FIELD;
            }
            break;
        case State::POST_FIELD:
            if (c == '=') {
                state = State::POST_VALUE;
            } else if (c == '&') {
                postFields[curPostField] = "";
                curPostField.clear();
            } else {
                curPostField.push_back(c);
            }
            contentBytesLeft--;
            if (contentBytesLeft == 0) {
                postFields[curPostField] = "";
                return false;
            }
            break;
        case State::POST_VALUE:
            if (c == '&') {
                postFields[curPostField] = curPostValue;
                curPostField.clear();
                curPostValue.clear();
                state = State::POST_FIELD;
            } else {
                curPostValue.push_back(c);
            }
            contentBytesLeft--;
            if (contentBytesLeft == 0) {
                postFields[curPostField] = curPostValue;
                return false;
            }
            break;
        }
    }
    return true;
}

void AuthServer::stop() {
    keepAlive = false;
    if (serverThread) {
        serverThread->join();
    }
}

AuthServer::~AuthServer() {
    stop();
}

} /* namespace navigraph */
