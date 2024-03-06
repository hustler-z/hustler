# tar.xz
- 将文件夹xxx压缩成tar.xz格式
```shell
tar -cvf xxx.tar xxx # 生成xxx.tar文件
xz -z -k 8 xxx.tar   # 生成xxx.tar.xz文件, 耗时较长，压缩效率高
tar -Jcvf xxx.tar.xz xxx
```

- 解压缩tar.xz文件
```shell
tar -xvjf xxx.tar.xz
```

- 拆分大文件
```shell
split -b 50M -d -a 2 xxx.tar.xz xxx.tar.xz.
```

- 合并文件
```shell
cat xxx.tar.xz.* > xxx.tar.xz
```


# tar.bz2
- 将文件夹xxx压缩成tar.bz2格式
```
tar -jcvf xxx.tar.bz2 xxx
```

- 解压bz2
```
tar -jxvf xxx.bz2
```

