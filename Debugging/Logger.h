//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_LOGGER_H
#define RELIC_2_0_LOGGER_H


#include <cstdarg>

class Logger
{
public:
    static void Log(int messageCount, ...);
    static void Log(const char* message);
};


#endif //RELIC_2_0_LOGGER_H
