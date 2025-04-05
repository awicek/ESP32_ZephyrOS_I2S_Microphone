cmake_minimum_required(VERSION 3.20.0)

set(BOARD esp32s3_devkitc/esp32s3/procpu)
# set(BOARD ttgo_lora32/esp32/procpu) 


set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/my.overlay) 
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(wifi_example)

target_sources(app PRIVATE src/main.cpp)
target_sources(app PRIVATE src/sound_recording.cpp)
target_sources(app PRIVATE src/access_point.cpp)
target_sources(app PRIVATE src/sound_queue.cpp)
target_sources(app PRIVATE src/network_com.cpp)

