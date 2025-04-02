#ifndef PCH_H
#define PCH_H

#include "framework.h"
#include "Defines.h"
#include <stdint.h>
#include <stdexcept>
#include <sstream>

#ifdef DLL_EXPORTS
#define DLL_SPEC __declspec(dllexport)
#else
#define DLL_SPEC __declspec(dllimport)
#endif




#endif //PCH_H
