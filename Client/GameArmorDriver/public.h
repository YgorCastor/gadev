#pragma once

#ifndef CTL_CODE
#include <winioctl.h>
#endif

#define GAMEARMOR_FILEIO_TYPE 0xFACA

#define IOCTL_GAMEARMOR_BOOT CTL_CODE(GAMEARMOR_FILEIO_TYPE, 0xA00, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
