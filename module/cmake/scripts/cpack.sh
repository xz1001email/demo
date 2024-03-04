
cd ../build
#生成二进制安装包
cpack -C CPackConfig.cmake

#生成源码安装包
#cpack -C CPackSourceConfig.cmake
