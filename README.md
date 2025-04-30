
# csync - remote sync file for c


使用c语言实现的远程同步。类似rsync的功能。
实现了增量同步的功能。
程序设计提供2个端口，分别用于管理（restapi）和数据同步（data）。
本程序基于mongoose、librsync、openssl等库。

- Cross-platform:
  - works on Linux, Windows  
- Built-in protocols: plain TCP

## 编译环境

windows: visual studio 2022

linux: fedora42



## 依赖的库

librsync

mongoose

openssl

json-c


## 开发内容

1. linux/windows 兼容。

2. 只能同步文件内容，暂未同步文件属性等。
  
3. 无加密
