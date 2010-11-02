/*! \file has_id.cpp
 *  \brief
 *  \author Gionata Massi <massi@diiga.univpm.it>
 */

#include "has_id.h"

has_id::has_id(const int id): _id(id)
{
}

has_id::~has_id()
{
}

int has_id::id() const
{
    return _id;
}

void has_id::id(const int id)
{
    _id = id;
}
