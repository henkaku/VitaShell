#pragma once

#include <psp2/types.h>

typedef struct {
	const char *file;
} InstallArguments;

int install_thread(SceSize args_size, InstallArguments *args);
