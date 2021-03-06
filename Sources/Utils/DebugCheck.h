#pragma once

#include <stdexcept>
#include "winerror.h"

#define DCHECK(condition) \
if (!(condition))\
{\
    throw std::runtime_error(#condition);\
}

#define DCHECK_COM(hr) DCHECK(SUCCEEDED(hr))
