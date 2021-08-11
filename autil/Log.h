#pragma once

#include <cstdio>

#define AUTIL_LOG_DECLARE()

#define AUTIL_LOG_SETUP(n, c)

#define AUTIL_DECLARE_AND_SETUP_LOGGER(n, c)

#define AUTIL_LOG(level, format, args...) printf(format, ##args)
