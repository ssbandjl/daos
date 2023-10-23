# CMake 最低版本号要求
cmake_minimum_required (VERSION 2.8)

# 项目信息
project (tse_test)


# 指定生成目标 
add_executable(tse_test tse_test.c)

# 添加链接库
target_link_libraries(tse_test cmocka)