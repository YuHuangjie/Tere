#ifndef CUDAUTILS_H
#define CUDAUTILS_H

#include <stdexcept>
#include <sstream>
#include "cuda_runtime.h"

#define CUDA_ERR_CHK(ans) { ThrowOnCudaErr((ans), __FILE__, __LINE__); }
inline void ThrowOnCudaErr(cudaError_t code, const char *file, int line)
{
	if (code != cudaSuccess) {
		std::stringstream ss;
		ss << "CUDA runtime error: " << cudaGetErrorString(code) 
			<< " at " << file << "(" << line << ")";
		throw std::runtime_error(ss.str());
	}
}

#endif /* CUDAUTILS_H */
