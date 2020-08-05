//
// Created by mikag on 3/31/2020.
//

#ifndef RELIC_RELICSTRUCT_H
#define RELIC_RELICSTRUCT_H

enum RelicType
{
    REL_TYPE_NONE,
    REL_TYPE_BINARY,
    REL_STRUCTURE_TYPE_INVALID,
    REL_STRUCTURE_TYPE_MODEL,
    REL_STRUCTURE_TYPE_MESH,
};

struct RelicStruct
{
    const RelicType sType = REL_STRUCTURE_TYPE_INVALID;
};

#endif //RELIC_RELICSTRUCT_H