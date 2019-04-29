#ifndef MEMORY_H
#define MEMORY_H

#define MEMCPY_HOST2HOST	0
#define MEMCPY_HOST2DEV		1
#define MEMCPY_DEV2HOST		2
#define MEMCPY_DEV2DEV		3

#define HOST2HOST(dst, src, sz) std::memcpy(dst, src, sz)

#ifdef USE_CUDA
	#include "cuda_runtime.h"
	#include "CudaUtils.h"

	#define HOST2DEV(dst, src, sz) CUDA_ERR_CHK(cudaMemcpy(dst, src, sz, cudaMemcpyHostToDevice))
	#define DEV2HOST(dst, src, sz) CUDA_ERR_CHK(cudaMemcpy(dst, src, sz, cudaMemcpyDeviceToHost))
	#define DEV2DEV(dst, src, sz) CUDA_ERR_CHK(cudaMemcpy(dst, src, sz, cudaMemcpyDeviceToDevice))
#else
	#define HOST2DEV(dst, src, sz) throw std::runtime_error("Invalid COPY: HOST2DEV")
	#define DEV2HOST(dst, src, sz) throw std::runtime_error("Invalid COPY: DEV2HOST")
	#define DEV2DEV(dst, src, sz)  throw std::runtime_error("Invalid COPY: DEV2DEV")
#endif /* USE_CUDA */

#define MEMCPY(dst, src, sz, mode) do {\
	switch (mode){\
		case MEMCPY_HOST2HOST: HOST2HOST(dst, src, sz); break;\
		case MEMCPY_HOST2DEV:  HOST2DEV(dst, src, sz); break;\
		case MEMCPY_DEV2HOST:  DEV2HOST(dst, src, sz); break;\
		case MEMCPY_DEV2DEV:   DEV2DEV(dst, src, sz); break;\
		default: throw std::runtime_error("Invalid MEMCPY mode"); break;\
	}\
} while(0)

#endif /* MEMORY_H */
