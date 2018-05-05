#include "MinerFactory.h"

#include <thread>

extern int use_avx;
extern int use_avx2;

MinerFactory::~MinerFactory()
{
	ClearAllSolvers();
}

std::vector<std::unique_ptr<ISolver>> MinerFactory::GenerateSolvers(int cpu_threads, int cuda_count, int* cuda_en, int* cuda_b, int* cuda_t,
	int opencl_count, int opencl_platf, int* opencl_en, int* opencl_t)
{
	std::vector<std::unique_ptr<ISolver>> solversPointers;

	for (int i = 0; i < cuda_count; ++i) {
		solversPointers.emplace_back( (ISolver*) GenCUDASolver(cuda_en[i], cuda_b[i], cuda_t[i]));
	}

	for (int i = 0; i < opencl_count; ++i)
	{
		if (opencl_t[i] < 1) opencl_t[i] = 1;

		// add multiple threads if wanted
		for (int k = 0; k < opencl_t[i]; ++k) {
			// todo: save local&global work size, new solvers
			solversPointers.emplace_back( (ISolver*) GenOPENCLSolver(opencl_platf, opencl_en[i]));
		}
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

ISolver * MinerFactory::GenCPUSolver(int use_opt) {
    // TODO fix dynamic linking on Linux
#ifdef    USE_CPU_XENONCAT
	if (_use_xenoncat)
	{
		return new CPUSolverXenoncat(use_opt);
	}
	else
	{
		return new CPUSolverTromp(use_opt);
	}
#else
	return new CPUSolverTromp(use_opt);
#endif
}

ISolver * MinerFactory::GenCUDASolver(int dev_id, int blocks, int threadsperblock) {
	if (_use_cuda_djezo) {
		return new CUDASolverDjezo(dev_id, blocks, threadsperblock);
	}
	else {
		return new CUDASolverTromp(dev_id, blocks, threadsperblock);
	}
}
// no OpenCL solvers at the moment keep for future reference
ISolver * MinerFactory::GenOPENCLSolver(int platf_id, int dev_id) {
	if (_use_silentarmy)
	{
		return new OPENCLSolverSilentarmy(platf_id, dev_id);
	}
	else
	{
		return new OPENCLSolverXMP(platf_id, dev_id);
	}
}
