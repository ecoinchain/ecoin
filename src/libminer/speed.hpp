#pragma once

#include <mutex>
#include <chrono>
#include <boost/circular_buffer.hpp>

#define INTERVAL_SECONDS 15 // 15 seconds

class Speed
{
	int m_interval;

	using time_point = std::chrono::high_resolution_clock::time_point;

	boost::circular_buffer<time_point> m_buffer_hashes;
	boost::circular_buffer<time_point> m_buffer_solutions;
	boost::circular_buffer<time_point> m_buffer_shares;
	boost::circular_buffer<time_point> m_buffer_shares_ok;

	std::mutex m_mutex_hashes;
	std::mutex m_mutex_solutions;
	std::mutex m_mutex_shares;
	std::mutex m_mutex_shares_ok;

	void Add(boost::circular_buffer<time_point>& buffer, std::mutex& mutex);
	double Get(boost::circular_buffer<time_point>& buffer, std::mutex& mutex);

public:
	Speed(int interval);
	virtual ~Speed();

	void AddHash();
	void AddSolution();
	void AddShare();
	void AddShareOK();
	double GetHashSpeed();
	double GetSolutionSpeed();
	double GetShareSpeed();
	double GetShareOKSpeed();

	void Reset();
};

extern Speed speed;
