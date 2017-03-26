/*
	VitaShell
	Copyright (C) 2015-2017, TheFloW

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"
#include "file.h"
#include "message_dialog.h"
#include "language.h"
#include "utils.h"

#define VITASHELL_VERSION_FILE "ux0:VitaShell/internal/version.bin"

void check_first_boot(void) {
	if (!is_molecular_shell) {
		return;
	}

	uint32_t not_first_boot = 0;
	ReadFile(henkaku_first_boot_path, &not_first_boot, sizeof(not_first_boot));
	if (is_safe_mode && !not_first_boot && dialog_step == DIALOG_STEP_NONE) {
		infoDialog(language_container[HENKAKU_FIRST_BOOT_MESSAGE]);

		not_first_boot = 1;
	} else if (!is_safe_mode) {
		not_first_boot = 1;
	}
	WriteFile(henkaku_first_boot_path, &not_first_boot, sizeof(not_first_boot));
}
