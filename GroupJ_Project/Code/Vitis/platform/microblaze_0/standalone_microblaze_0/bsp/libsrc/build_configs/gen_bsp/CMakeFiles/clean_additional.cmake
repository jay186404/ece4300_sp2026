# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "C:\\Users\\user\\Desktop\\Projects\\ECE4300_SP2026\\GroupJ_Project\\Code\\Vitis\\platform\\microblaze_0\\standalone_microblaze_0\\bsp\\include\\sleep.h"
  "C:\\Users\\user\\Desktop\\Projects\\ECE4300_SP2026\\GroupJ_Project\\Code\\Vitis\\platform\\microblaze_0\\standalone_microblaze_0\\bsp\\include\\xiltimer.h"
  "C:\\Users\\user\\Desktop\\Projects\\ECE4300_SP2026\\GroupJ_Project\\Code\\Vitis\\platform\\microblaze_0\\standalone_microblaze_0\\bsp\\include\\xtimer_config.h"
  "C:\\Users\\user\\Desktop\\Projects\\ECE4300_SP2026\\GroupJ_Project\\Code\\Vitis\\platform\\microblaze_0\\standalone_microblaze_0\\bsp\\lib\\libxiltimer.a"
  )
endif()
