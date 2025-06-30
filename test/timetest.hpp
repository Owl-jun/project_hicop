#pragma once
#include <chrono>
#include <iostream>
class timetest {
private:
	timetest() {};
	~timetest() {};
public:
	std::chrono::steady_clock::time_point start;
	std::chrono::steady_clock::time_point end;
	std::chrono::duration<double> duration;

	static timetest& GetInstance() {
		static timetest instance;
		return instance;
	}

	void go() {
		start = std::chrono::steady_clock::now();
	}
	void stop() {
		end = std::chrono::steady_clock::now();
		duration = end - start;
		//std::cout << "경과시간 : " << duration.count() << std::endl;
	}

};