//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_LOGGER_H
#define RELIC_2_0_LOGGER_H

class Logger
{
private:
    static char * Convert(unsigned int num, int base);

public:
    static void Log(const char* format, ...);
};


#endif //RELIC_2_0_LOGGER_H
