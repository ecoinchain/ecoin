#pragma once

#include "Solver.h"
#include "SolverStub.h"

#include "cpu_tromp/cpu_tromp.hpp"

#ifdef USE_CUDA_TROMP
#include "cuda_tromp/cuda_tromp.hpp"
#else
CREATE_SOLVER_STUB(cuda_tromp, "cuda_tromp_STUB")
#endif

#ifdef USE_OCL_SILENTARMY
#include "ocl_silentarmy/ocl_silentarmy.hpp"
#else
CREATE_SOLVER_STUB(ocl_silentarmy, "ocl_silentarmy_STUB")
#endif

//namespace AvailableSolvers
//{
//} // AvailableSolvers

// CPU solvers
class CPUSolverTromp : public Solver<cpu_tromp> {
public:
	CPUSolverTromp(int use_opt) : Solver<cpu_tromp>(new cpu_tromp(), SolverType::CPU) {
		_context->use_opt = use_opt;
	}
	virtual ~CPUSolverTromp() {}
};

class CUDASolverTromp : public Solver<cuda_tromp> {
public:
	CUDASolverTromp(int dev_id, int blocks, int threadsperblock) : Solver<cuda_tromp>(new cuda_tromp(0, dev_id), SolverType::CUDA) {
		if (blocks > 0) {
			_context->blocks = blocks;
		}
		if (threadsperblock > 0) {
			_context->threadsperblock = threadsperblock;
		}
	}
	virtual ~CUDASolverTromp() {}
};
// OpenCL solvers
class OPENCLSolverSilentarmy : public Solver<ocl_silentarmy> {
public:
	OPENCLSolverSilentarmy(int platf_id, int dev_id) : Solver<ocl_silentarmy>(new ocl_silentarmy(platf_id, dev_id), SolverType::OPENCL) {
	}
	virtual ~OPENCLSolverSilentarmy() {}
};
