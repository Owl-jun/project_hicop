#pragma once

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include <thread>
#include <vector>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <unordered_map>
#include <string>
#include <istream>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <mswsock.h>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

using json = nlohmann::json;