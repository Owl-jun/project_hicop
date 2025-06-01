#pragma once
#include "pch.h"

namespace fileutils {
	inline short getPortByJson() {
		std::ifstream file("jsons/port.json");
		if (!file.is_open()) {
			std::cerr << "파일을 열 수 없습니다.\n";
			return 1;
		}

		json j;
		file >> j;

		short port = j["port"];

		return port;
	}
}

