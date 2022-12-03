/*
 * sha256.cuh CUDA Implementation of SHA256 Hashing    
 *
 * Date: 12 June 2019
 * Revision: 1
 * 
 * Based on the public domain Reference Implementation in C, by
 * Brad Conte, original code here:
 *
 * https://github.com/B-Con/crypto-algorithms
 *
 * This file is released into the Public Domain.
 */


#pragma once
#include "config.h"
#include "../merkle_tree.hpp"
void mcm_cuda_sha256_hash_batch(BYTE* in, WORD inlen, BYTE* out, WORD n_batch);
__global__ void kernel_sha256_hash(BYTE* indata, WORD inlen, BYTE* outdata, WORD n_batch);
__global__ void kernel_sha256_hash_cont(BYTE* indata, WORD inlen, BYTE* outdata, WORD n_batch);
__global__ void kernel_sha256_hash_link(BYTE* indata, WORD inlen,
                                        BYTE* outdata, WORD n_batch,
                                        BYTE* dout,
                                        unsigned int* dparents,
                                        unsigned int* dlefts,
                                        unsigned int* drights,
                                        LeftOrRightSib* dlrs);
__global__ void kernel_link_merklenode(BYTE* hashes, WORD hash_size,
                                       unsigned int* dparent,
                                       unsigned int* dlefts,
                                       unsigned int* drights,
                                       LeftOrRightSib* dlrs,
                                       MerkleNode* nodes,
                                       MerkleNode* dnodes, WORD n_nodes,
                                       WORD num_of_leaves);
