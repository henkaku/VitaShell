cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_C_COMPILER "arm-vita-eabi-gcc")
set(CMAKE_CXX_COMPILER "arm-vita-eabi-g++")

project(VitaShell)

set(CMAKE_C_FLAGS "-Wl,-q -Wall -O3 -Wno-unused-variable -Wno-unused-but-set-variable")
set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

file(GLOB_RECURSE FEX_C_SRCS fex/*.c)
file(GLOB_RECURSE FEX_CPP_SRCS fex/*.cpp)

include_directories(
	fex
	fex/7z_C
	fex/fex
	fex/unrar

	libpromoter
)

link_directories(
	${CMAKE_CURRENT_BINARY_DIR}
)

include(ExternalProject)
ExternalProject_Add(libpromoter
	SOURCE_DIR ${CMAKE_SOURCE_DIR}/libpromoter
	CONFIGURE_COMMAND ""
	BUILD_IN_SOURCE 1
	BUILD_COMMAND make
	PREFIX=${CMAKE_CURRENT_BINARY_DIR}
	INSTALL_COMMAND ""
)

set(RESOURCES "")
function(BuildResource arg)
	set_source_files_properties(
		${CMAKE_CURRENT_BINARY_DIR}/${arg}.o
		PROPERTIES GENERATED ON
	)

	add_custom_command(
		OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${arg}.o
		COMMAND arm-vita-eabi-ld -r -b binary -o ${CMAKE_CURRENT_BINARY_DIR}/${arg}.o resources/${arg}
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	)

	set(RESOURCES ${RESOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${arg}.o PARENT_SCOPE)
endfunction()

BuildResource(battery.png)
BuildResource(battery_bar_green.png)
BuildResource(battery_bar_red.png)
BuildResource(english_us_translation.txt)

add_executable(VitaShell.elf
	archive.c
	file.c
	hex.c
	init.c
	io_process.c
	language.c
	main.c
	message_dialog.c
	misc.c
	package_installer.c
	photo.c
	sha1.c
	text.c
	uncommon_dialog.c
	utils.c

	${FEX_C_SRCS}
	${FEX_CPP_SRCS}

	${RESOURCES}
)

target_link_libraries(VitaShell.elf
	ftpvita
	vita2d
	jpeg
	png
	z

	SceAppMgr_stub
	SceAppUtil_stub
	SceCtrl_stub
	SceDisplay_stub
	SceGxm_stub
	SceNet_stub
	SceNetCtl_stub
	ScePgf_stub
	ScePower_stub
	SceSysmodule_stub

	ScePromoterUtil_stub
)
add_dependencies(VitaShell.elf libpromoter)

add_custom_target(VitaShell.bin ALL
	COMMAND vita-elf-create VitaShell.elf VitaShell.velf ${CMAKE_SOURCE_DIR}/libpromoter/promoterutil.json
	COMMAND vita-make-fself VitaShell.velf VitaShell.bin
)
add_dependencies(VitaShell.bin VitaShell.elf)
