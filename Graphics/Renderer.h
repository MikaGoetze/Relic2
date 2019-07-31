//
// Created by Mika Goetze on 2019-07-31.
//

#ifndef RELIC_RENDERER_H
#define RELIC_RENDERER_H


class Renderer
{
public:
    virtual void Initialise() = 0;

    virtual void Cleanup() = 0;
};


#endif //RELIC_RENDERER_H
