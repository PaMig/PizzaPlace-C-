
//incl src file once
#pragma once

//rest of includes
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <time.h>

//get in nanoseconds

#define BILLION 1000000000
//constants

#define Ntel 3
#define Ncook 2
#define Noven 10
#define Ndeliverer 7
#define Npacker 1

const static int Torderlow = 1;
const static int Torderhigh = 5;
const static int Norderlow = 1;
const static int Norderhigh = 5;
const static int Tpaymentlow = 1;
const static int Tpaymenthigh = 2;
const static int Tprep = 1 ;
const static int Tpack = 2;
const static int Tbake = 10;
const static int Tdellow = 5;
const static int Tdelhigh = 15;
const static double Cpay = 0.5;//unused

void sync_print(const char *format, ...);

double dtime(struct timespec* before,struct timespec* after);

void * order_thread(void *x);
