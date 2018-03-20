# 加入测试网络

* 下载源码到本地目录
```bash
git clone https://github.com/rcoinchain/rcoin.git
```

* 编译源码

如果是mac系统，请参考rcoin/doc目录下的build-osx.md，其它类似的操作系统对应的
build说明文档

注意在执行.configure时暂时先不要编译test源码
```bash
./configure BDB_LIBS="-L${BDB_PREFIX}/lib -ldb_cxx-4.8" BDB_CFLAGS="-I${BDB_PREFIX}/include" --without-gui --disable-tests --disable-gui-tests --disable-bench
```

* 设置bitcoin.conf
```
# Run on the test network instead of the real bitcoin network.
testnet=1

# Use as many addnode= settings as you like to connect to specific peers
addnode=59.110.153.149
addnode=60.205.188.43
addnode=47.97.121.28
addnode=47.97.201.211

# If not, you must set rpcuser and rpcpassword to secure the JSON-RPC api. The first
# method(DEPRECATED) is to set this pair for the server and client:
rpcuser=your rpc user
rpcpassword=your rpc password
rpcport=18875
```
```59.110.153.149,60.205.188.43,47.97.121.28,47.97.201.211```这四个节点是启动网络的种子节点

* 启动程序
```bash
/path/rcoin/src/bitcoind -datadir=/path/your datadir -daemon
```