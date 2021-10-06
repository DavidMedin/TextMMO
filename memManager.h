#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
nng_mtx* startRMMut;// start resource manager mutex
nng_mtx* endRMMut;
nng_mtx* totalRMMut;

nng_cv* startRMVar;

nng_thread* memThread;
int StartAllocator();
void KillAllocator();
void* RequestMemory(unsigned int size);
void FreeMemory(void* mem);