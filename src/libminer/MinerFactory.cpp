#include "MinerFactory.h"

#include <thread>

extern int use_avx;
extern int use_avx2;

MinerFactory::~MinerFactory()
{
	ClearAllSolvers();
}

std::vector<std::unique_ptr<ISolver>> MinerFactory::GenerateSolvers(int cpu_threads, int cuda_count, int* cuda_en, int* cuda_b, int* cuda_t,
	int opencl_count, int opencl_platf, int* opencl_en)
{
	std::vector<std::unique_ptr<ISolver>> solversPointers;

	for (int i = 0; i < cuda_count; ++i) {
		solversPointers.emplace_back( (ISolver*) GenCUDASolver(cuda_en[i], cuda_b[i], cuda_t[i]));
	}

	for (int i = 0; i < opencl_count; ++i)
	{
		solversPointers.emplace_back( (ISolver*) GenOPENCLSolver(opencl_platf, opencl_en[i]));
	}

	bool hasGpus = solversPointers.size() > 0;
	if (cpu_threads < 0) {
		cpu_threads = std::thread::hardware_concurrency();
		if (cpu_threads < 1) cpu_threads = 1;
		else if (hasGpus) --cpu_threads; // decrease number of threads if there are GPU workers
	}

	for (int i = 0; i < cpu_threads; ++i)
	{
		solversPointers.emplace_back((ISolver*) GenCPUSolver(use_avx2));
	}

	return solversPointers;
}

void MinerFactory::ClearAllSolvers()
{
}

ISolver * MinerFactory::GenCPUSolver(int use_opt)
{
	return new CPUSolverTromp(use_opt);
}

ISolver * MinerFactory::GenCUDASolver(int dev_id, int blocks, int threadsperblock)
{
	return new CUDASolverDjezo(dev_id, blocks, threadsperblock);
}
// no OpenCL solvers at the moment keep for future reference
ISolver * MinerFactory::GenOPENCLSolver(int platf_id, int dev_id)
{
	return new OPENCLSolverSilentarmy(platf_id, dev_id);
}
