/*
 *   AviTab - Aviator's Virtual Tablet
 *   Copyright (C) 2024 Folke Will <folko@solhost.org>
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
#pragma once

#include <memory>
#include <atomic>
#include <thread>
#include <string>
#include <map>
#include <functional>

namespace chartfox {

class ChartFoxAuthServer {
public:
    using AuthCallback = std::function<void(const std::string &)>;

    ChartFoxAuthServer();
    void setAuthCallback(AuthCallback cb);
    int start(); // returns port
    void stop();
    virtual ~ChartFoxAuthServer();
private:
    enum class State {METHOD, URL, VERSION, FIELD, VALUE, POST_FIELD, POST_VALUE};

    AuthCallback onAuth;

    int srvSock = -1;
    std::atomic_bool keepAlive { false };
    std::unique_ptr<std::thread> serverThread;

    State state = State::METHOD;
    std::string method, url, version;
    std::string curHeaderField, curHeaderValue;
    std::string curPostField, curPostValue;
    std::map<std::string, std::string> postFields;
    size_t contentBytesLeft = 0;

    void loop();
    void handleClient(int client);
    bool handleData(const std::string& data);
};

} /* namespace chartfox */
