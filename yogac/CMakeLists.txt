cmake_minimum_required(VERSION 3.9)
aux_source_directory(./ SRC_LIST)
aux_source_directory(./yoga YOGA_SRC_LIST)
list(APPEND SRC_LIST ${YOGA_SRC_LIST})


add_executable(hello ${SRC_LIST})

MESSAGE("正在生成项目文件")
