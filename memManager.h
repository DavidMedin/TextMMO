#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <nng/nng.h>
#include <nng/supplemental/util/platform.h>
nng_mtx* mtx;
nng_cv* condiVar;
nng_thread* memThread;
int StartAllocator();
void KillAllocator();
void* RequestMemory(unsigned int size);
void FreeMemory(void* mem);