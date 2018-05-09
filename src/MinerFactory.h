#pragma once

#include <memory>
#include "AvailableSolvers.h"

class MinerFactory
{
public:
	~MinerFactory();

	std::vector<std::unique_ptr<ISolver>> GenerateSolvers(int cpu_threads, int cuda_count, int* cuda_en, int* cuda_b, int* cuda_t,
		int opencl_count, int opencl_platf, int* opencl_en, int* opencl_t);
	void ClearAllSolvers();

private:
	ISolver * GenCPUSolver(int use_opt);
	ISolver * GenCUDASolver(int dev_id, int blocks, int threadsperblock);
	ISolver * GenOPENCLSolver(int platf_id, int dev_id);
};

