# C/S 模型：TCP 和 UDP 流程

C/S 是 Client/Server，也就是客户端/服务端模型。

本目录里有两组示例：

```text
tcp_server.cpp  TCP 服务端
tcp_client.cpp  TCP 客户端
udp_server.cpp  UDP 服务端
udp_client.cpp  UDP 客户端
```

## 1. 编译和运行

编译：

```bash
cmake -S cs -B cs/build
cmake --build cs/build
```

TCP 测试：

```bash
./cs/build/tcp_server
./cs/build/tcp_client
```

UDP 测试：

```bash
./cs/build/udp_server
./cs/build/udp_client
```

测试时先运行 server，再打开另一个终端运行 client。

## 2. TCP 是什么

TCP 是面向连接的可靠传输协议。

特点：

```text
有连接
可靠传输
保证数据顺序
有重传机制
有流量控制和拥塞控制
```

适合：

```text
网页 HTTP/HTTPS
登录系统
聊天消息
文件传输
数据库连接
```

TCP 像打电话：先建立连接，再持续通信，最后断开连接。

## 3. TCP 服务端流程

TCP 服务端流程：

```text
socket
  -> bind
  -> listen
  -> accept
  -> recv/send
  -> close
```

对应代码思想：

```cpp
int server_fd = socket(AF_INET, SOCK_STREAM, 0);
bind(server_fd, ...);
listen(server_fd, 5);

int client_fd = accept(server_fd, ...);
recv(client_fd, ...);
send(client_fd, ...);

close(client_fd);
close(server_fd);
```

每一步含义：

| 函数 | 作用 |
|---|---|
| `socket()` | 创建 TCP socket |
| `bind()` | 绑定服务端 IP 和端口 |
| `listen()` | 开始监听客户端连接 |
| `accept()` | 接受一个客户端连接，得到新的 `client_fd` |
| `recv()` | 从客户端接收数据 |
| `send()` | 向客户端发送数据 |
| `close()` | 关闭 socket |

注意：

```text
server_fd 负责监听
client_fd 负责和某个客户端通信
```

`accept()` 成功后会返回一个新的 socket，这个新的 `client_fd` 才是真正用于收发数据的连接。

## 4. TCP 客户端流程

TCP 客户端流程：

```text
socket
  -> connect
  -> send/recv
  -> close
```

对应代码思想：

```cpp
int client_fd = socket(AF_INET, SOCK_STREAM, 0);
connect(client_fd, ...);

send(client_fd, ...);
recv(client_fd, ...);

close(client_fd);
```

每一步含义：

| 函数 | 作用 |
|---|---|
| `socket()` | 创建 TCP socket |
| `connect()` | 连接服务端 IP 和端口 |
| `send()` | 向服务端发送数据 |
| `recv()` | 接收服务端回复 |
| `close()` | 关闭连接 |

## 5. UDP 是什么

UDP 是无连接的数据报协议。

特点：

```text
无连接
不保证可靠
不保证顺序
没有 accept
没有 listen
开销小
速度快
```

适合：

```text
实时语音
实时视频
游戏同步
广播
局域网发现
DNS 查询
```

UDP 像寄快递：每个数据包都带着目标地址，直接发出去。至于对方有没有收到、顺序是不是一致，UDP 本身不保证。

## 6. UDP 服务端流程

UDP 服务端流程：

```text
socket
  -> bind
  -> recvfrom
  -> sendto
  -> close
```

对应代码思想：

```cpp
int server_fd = socket(AF_INET, SOCK_DGRAM, 0);
bind(server_fd, ...);

recvfrom(server_fd, ..., &client_addr, &client_len);
sendto(server_fd, ..., &client_addr, client_len);

close(server_fd);
```

每一步含义：

| 函数 | 作用 |
|---|---|
| `socket()` | 创建 UDP socket |
| `bind()` | 绑定服务端 IP 和端口 |
| `recvfrom()` | 接收数据，并拿到发送方地址 |
| `sendto()` | 按指定地址发送数据 |
| `close()` | 关闭 socket |

UDP 服务端没有 `listen()` 和 `accept()`。

原因是 UDP 不需要建立连接，服务端只要绑定端口，然后等待别人发数据过来即可。

## 7. UDP 客户端流程

UDP 客户端流程：

```text
socket
  -> sendto
  -> recvfrom
  -> close
```

对应代码思想：

```cpp
int client_fd = socket(AF_INET, SOCK_DGRAM, 0);

sendto(client_fd, ..., &server_addr, sizeof(server_addr));
recvfrom(client_fd, ..., &from_addr, &from_len);

close(client_fd);
```

每一步含义：

| 函数 | 作用 |
|---|---|
| `socket()` | 创建 UDP socket |
| `sendto()` | 向服务端地址发送数据 |
| `recvfrom()` | 接收服务端回复 |
| `close()` | 关闭 socket |

客户端通常不需要手动 `bind()`，操作系统会自动分配一个临时端口。

## 8. TCP 和 UDP 对比

| 对比项 | TCP | UDP |
|---|---|---|
| 连接 | 有连接 | 无连接 |
| socket 类型 | `SOCK_STREAM` | `SOCK_DGRAM` |
| 服务端是否 listen | 需要 | 不需要 |
| 服务端是否 accept | 需要 | 不需要 |
| 发送函数 | `send()` | `sendto()` |
| 接收函数 | `recv()` | `recvfrom()` |
| 可靠性 | 可靠 | 不可靠 |
| 顺序 | 保证顺序 | 不保证顺序 |
| 速度 | 相对慢一些 | 相对快一些 |
| 常见用途 | 文件、网页、聊天 | 语音、视频、游戏、DNS |

## 9. 地址结构 sockaddr_in

IPv4 常用 `sockaddr_in`：

```cpp
sockaddr_in addr{};
addr.sin_family = AF_INET;
addr.sin_port = htons(8080);
addr.sin_addr.s_addr = INADDR_ANY;
```

字段含义：

| 字段 | 含义 |
|---|---|
| `sin_family` | 地址族，IPv4 使用 `AF_INET` |
| `sin_port` | 端口，需要用 `htons()` 转成网络字节序 |
| `sin_addr` | IP 地址 |

常用 IP 写法：

```cpp
server_addr.sin_addr.s_addr = INADDR_ANY;
```

表示服务端绑定本机所有网卡地址。

客户端连接本机服务端：

```cpp
inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);
```

`127.0.0.1` 表示本机回环地址。

## 10. 常见返回值

`recv()` 或 `recvfrom()`：

```text
返回值 > 0：收到的字节数
返回值 = 0：TCP 对端正常关闭连接
返回值 = -1：出错
```

UDP 的 `recvfrom()` 一般不会因为“对端关闭”返回 0，因为 UDP 没有连接状态。

`send()` 或 `sendto()`：

```text
返回值 >= 0：实际发送的字节数
返回值 = -1：出错
```

## 11. 初学记忆

TCP 记忆：

```text
服务端：socket bind listen accept recv send close
客户端：socket connect send recv close
```

UDP 记忆：

```text
服务端：socket bind recvfrom sendto close
客户端：socket sendto recvfrom close
```

最关键区别：

```text
TCP 先建立连接，再通信。
UDP 不建立连接，直接按地址发数据包。
```
