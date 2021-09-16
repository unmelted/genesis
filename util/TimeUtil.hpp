/*
 *  2019.10.01 created by Kelly @nepes
 *  UTIL FOR TIMEER
 *  TimeUtil.hpp
 *  Copyright (c) 2019, nepes inc, All rights reserved.
 *
 */
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <stdarg.h>
#include <chrono>
#include "DefData.hpp"

void InitTimer(TIMER *times);
void SetTimer(TIMER *times, unsigned int expire, void (*func)(void *), void *arg, int id);
long long GetTime();
void PassTime();

void StartTimer(TIMER *times);
int EndTimer(TIMER *times);
int LapTimer(TIMER *times);

std::string getSerial();
std::string getCurrentDateTime( std::string s );
void Logger(const char *format, ...);
