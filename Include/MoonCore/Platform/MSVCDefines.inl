#pragma once
#include <vcruntime_new.h>

#define LIBRARY_IMPORT __declspec(dllimport)
#define LIBRARY_EXPORT __declspec(dllexport)

#pragma warning(disable:	0501)
#pragma warning(disable:	4091 4251 4506 4828 4996)
#pragma warning(disable:	5103 5202 5244 )

#pragma warning(disable:	26498 26800)