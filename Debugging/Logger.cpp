//
// Created by mikag on 9/30/2018.
//

#include <iostream>
#include "Logger.h"

void Logger::Log(int messageCount, ...)
{
    va_list args;
    va_start(args, messageCount);
    for (int i = 0; i < messageCount; i++)
    {
        std::cout << va_arg(args, char*);
    }
    std::cout << std::endl;
    va_end(args);
}

void Logger::Log(const char *message)
{
    Log(1, message);
}
