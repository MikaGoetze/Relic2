//
// Created by mikag on 9/30/2018.
//

#ifndef RELIC_2_0_UTIL_H
#define RELIC_2_0_UTIL_H

#ifdef _MSC_VER
#define PAUSE()\
    __asm pause;
#else
#define PAUSE()\
    __asm__("pause;");
#endif

#endif //RELIC_2_0_UTIL_H
