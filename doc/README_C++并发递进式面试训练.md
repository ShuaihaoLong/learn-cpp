# C++ 并发递进式面试训练

> 面向嵌入式 Linux / C++ 秋招\
> 学习路径：`mutex → condition_variable → future → ThreadPool → Async Session → lock-free`

## 使用方法

-   每道题先口述 **2--5 分钟**，再写核心代码；不要先看"考察点"。
-   回答结构优先采用：**问题是什么 → 为什么 → 正确设计 → 边界情况 →
    工程取舍**。
-   不会时记录具体断点，例如"知道 `condition_variable`，但说不清 `wait`
    为什么必须配 `mutex`"，不要只记"并发不会"。
-   完成第 1--7 题后再学习无锁队列，否则容易停留在 `compare_exchange`
    的代码记忆层。

------------------------------------------------------------------------

## 第 1 题：mutex 与数据竞争

有两个线程同时执行：

``` cpp
int counter = 0;

void worker() {
    for (int i = 0; i < 100000; ++i)
        ++counter;
}
```

请回答：

1.  最终 `counter` 是否一定为 `200000`？
2.  如果不是，解释 `++counter` 为什么不是原子操作。
3.  分别使用 `mutex` 和 `atomic` 修改。
4.  `mutex` 和 `atomic` 各自适用于什么场景？

### 面试官考察点

-   data race / undefined behavior
-   read-modify-write
-   临界区
-   `std::atomic`
-   `mutex` 与 `atomic` 的适用范围

### 自测

-   [ ] 能独立回答
-   [ ] 能写出代码
-   [ ] 能解释为什么
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 2 题：RAII 与锁管理

下面代码有什么风险？

``` cpp
mutex.lock();
doSomething();
mutex.unlock();
```

继续回答：

1.  如果 `doSomething()` 抛异常会发生什么？
2.  `std::lock_guard`、`std::unique_lock`、`std::scoped_lock`
    有什么区别？
3.  为什么 `condition_variable` 通常使用 `unique_lock` 而不是
    `lock_guard`？
4.  多个 mutex 同时加锁时，如何降低死锁风险？

### 面试官考察点

-   RAII
-   异常安全
-   `unique_lock` 可解锁 / 重新加锁
-   多 mutex 死锁规避

### 自测

-   [ ] 能独立回答
-   [ ] 能解释三种锁包装器区别
-   [ ] 能联系 RAII
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 3 题：condition_variable

实现一个线程安全：

``` cpp
template<typename T>
class BlockingQueue;
```

至少支持：

``` cpp
void push(T value);
T pop();
void stop();
```

要求：

1.  `pop()` 在队列为空时阻塞。
2.  `stop()` 后唤醒所有消费者。
3.  正确处理虚假唤醒。
4.  明确 `stop()` 后队列中剩余元素是继续消费还是直接丢弃。

然后解释：

``` cpp
cv.wait(lock, pred);
```

内部大致是什么逻辑？

进一步回答：

-   为什么 `condition_variable` 要配合 mutex？
-   什么是虚假唤醒？
-   什么是 lost wakeup？
-   `notify_one()` 和 `notify_all()` 怎么选？
-   shutdown 时为什么通常需要 `notify_all()`？

### 面试官考察点

-   predicate
-   spurious wakeup
-   lost wakeup
-   `notify_one` / `notify_all`
-   shutdown 语义

### 自测

-   [ ] 能从零写 BlockingQueue
-   [ ] 能解释 wait
-   [ ] 能解释虚假唤醒
-   [ ] 能处理 shutdown
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 4 题：死锁

线程 A：

``` text
lock(mutex1)
lock(mutex2)
```

线程 B：

``` text
lock(mutex2)
lock(mutex1)
```

请回答：

1.  为什么可能产生死锁？
2.  死锁形成的必要条件是什么？
3.  给出至少三种解决办法。
4.  `std::scoped_lock(m1, m2)` 的意义是什么？
5.  如果业务必须分阶段获取锁，如何制定 lock ordering？

### 面试官考察点

-   互斥
-   持有并等待
-   不可抢占
-   循环等待
-   固定加锁顺序
-   `std::lock`
-   `std::scoped_lock`
-   缩小临界区

### 自测

-   [ ] 能说出死锁条件
-   [ ] 能给出工程解决方案
-   [ ] 能解释 scoped_lock
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 5 题：promise / future

设计：

``` cpp
std::future<int> asyncCalculate();
```

调用者立即获得 `future`，工作线程稍后完成计算并通过 `promise` 返回结果。

回答：

1.  `promise` 和 `future` 分别代表什么？
2.  `set_value()` 之前调用 `future.get()` 会发生什么？
3.  `promise` 被销毁但没有 `set_value()` 会怎样？
4.  `future.get()` 能调用几次？
5.  `shared_future` 解决什么问题？
6.  `promise/future` 背后的 shared state 是什么？

### 面试官考察点

-   shared state
-   阻塞等待
-   `broken_promise`
-   future 一次性消费
-   `shared_future`

### 自测

-   [ ] 能写 promise/future 示例
-   [ ] 理解 shared state
-   [ ] 理解异常传播
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 6 题：线程池

设计一个最小 `ThreadPool`：

``` cpp
class ThreadPool {
public:
    explicit ThreadPool(size_t n);

    template<class F, class... Args>
    auto submit(F&& f, Args&&... args);

    ~ThreadPool();
};
```

要求：

-   固定 N 个 worker。
-   `submit()` 支持不同 callable。
-   `submit()` 返回 `future`。
-   worker 从任务队列获取任务。
-   析构时安全停止。

继续回答：

1.  任务队列里面存什么类型？
2.  不同 callable 返回值不同，怎么统一存储？
3.  `std::packaged_task` 有什么作用？
4.  为什么经常使用 `std::function<void()>` 做类型擦除？
5.  `submit(F&& f, Args&&... args)` 为什么涉及完美转发？
6.  析构时是丢弃剩余任务还是执行完成？如何设计？

### 面试官考察点

-   BlockingQueue
-   `packaged_task`
-   future
-   type erasure
-   `std::function`
-   perfect forwarding
-   shutdown

### 自测

-   [ ] 能从零实现基础 ThreadPool
-   [ ] 能解释 packaged_task
-   [ ] 能解释完美转发的用途
-   [ ] 能设计 shutdown
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 7 题：异步 SerialSession

多个业务线程通过同一个 UART 与 MCU 通信。

设计接口：

``` cpp
std::future<Response> asyncRequest(Request req);
```

要求：

-   底层通信线程不能阻塞等待自己的 `future`。
-   多线程可以并发提交请求。
-   UART 实际发送必须串行。
-   Response 能匹配正确的 Request。

尝试画出：

``` text
Business Threads
       │
       ↓
 asyncRequest()
       │
       ↓
 Request Queue
       │
       ↓
 SerialSession
    │       ↑
 encode   decode
    │       │
    ↓       │
 UART ─── MCU
            │
            ↓
       sequence_id
            │
            ↓
       pending_map
            │
            ↓
   promise.set_value()
```

回答：

1.  为什么底层 Session 更适合异步？
2.  为什么业务线程可以 `future.get()`，通信线程却不能？
3.  `sequence_id` 有什么作用？
4.  `pending_map` 应该保存什么？
5.  如果底层协议根本没有 sequence ID 怎么办？
6.  为什么一个物理 UART 更适合由一个 Session 统一 ownership？

### 面试官考察点

-   生产者 / 消费者
-   单线程 IO ownership
-   sequence ID / correlation ID
-   pending map
-   promise / future

### 自测

-   [ ] 能画完整架构
-   [ ] 能解释 ownership
-   [ ] 能设计请求响应匹配
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 8 题：超时、迟到响应与重试

在第 7 题基础上加入：

``` text
timeout = 500ms
```

回答：

1.  `PendingRequest` 如何记录 deadline？
2.  为什么 timeout 通常使用 `std::chrono::steady_clock`？
3.  谁负责检查 timeout？
4.  timeout 后如何让 `future` 得到错误？
5.  timeout 后旧 Response 又到达了怎么办？
6.  哪些请求适合 retry？
7.  哪些请求不能无脑 retry？
8.  "电机前进 1 米"发生 timeout 后直接重试可能产生什么问题？
9.  request ID 去重可以解决什么问题？

### 面试官考察点

-   `steady_clock`
-   `set_exception`
-   stale response
-   幂等性
-   request ID 去重
-   at-most-once / at-least-once 思想

### 自测

-   [ ] 能设计 timeout
-   [ ] 理解 late response
-   [ ] 理解幂等性
-   [ ] 不会无脑回答"超时重试三次"
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 9 题：连接断开与状态机

`SerialSession` 工作过程中 UART 断开，3 秒后恢复。

设计状态机，例如：

``` text
Disconnected
     ↓
 Connecting
     ↓
 Connected
     ↓
 Error / Retry
```

讨论：

1.  已发送但未响应的请求怎么办？
2.  尚未发送的队列是否保留？
3.  reconnect 是否需要退避？
4.  reconnect 后旧 pending request 是否还能继续？
5.  如何避免多个线程同时执行重连？
6.  如何区分：
    -   请求尚未发送
    -   请求已经发送但结果未知
7.  为什么非幂等请求不能透明重试？

### 面试官考察点

-   状态机
-   ownership
-   错误传播
-   backoff
-   single-flight reconnect
-   恢复策略

### 自测

-   [ ] 能画状态机
-   [ ] 能区分未发送和结果未知
-   [ ] 能设计错误传播
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

## 第 10 题：从锁到无锁

假设 `BlockingQueue` 已经成为性能瓶颈，你准备改成 lock-free queue。

**先不要写代码。**

回答：

1.  CAS 是什么？
2.  `compare_exchange_weak` 为什么可能失败？
3.  acquire / release 分别约束什么？
4.  ABA 问题是什么？
5.  一个节点从队列摘除后为什么不能立刻 `delete`？
6.  hazard pointer / epoch reclamation 大致解决什么问题？
7.  lock-free 是否等于 wait-free？
8.  你的业务真的需要无锁吗？
9.  应该怎样 benchmark，证明 mutex queue 已经成为瓶颈？

### 面试官考察点

-   CAS
-   memory ordering
-   ABA
-   memory reclamation
-   hazard pointer / epoch
-   benchmark before optimization

### 自测

-   [ ] 理解 CAS
-   [ ] 能基本解释 acquire/release
-   [ ] 理解 ABA
-   [ ] 理解 memory reclamation 问题
-   [ ] 知道为什么不能为了"高级"而无锁
-   卡住的知识点：
-   二刷日期：

------------------------------------------------------------------------

# 推荐训练顺序

``` text
第 1–4 题
mutex / RAII / condition_variable / deadlock
            ↓
第 5–6 题
promise / future / ThreadPool
            ↓
第 7–9 题
Async Session / timeout / reconnect
            ↓
第 10 题
atomic / memory_order / lock-free
```

## 达标标准

### 第一阶段：第 1--4 题

能够：

-   独立解释 mutex、condition_variable。
-   从零实现 BlockingQueue。
-   找出常见 data race。
-   解释并解决基本死锁。

### 第二阶段：第 5--6 题

能够：

-   正确使用 promise / future。
-   从零实现基础 ThreadPool。
-   理解 `packaged_task`。
-   理解 callable type erasure。
-   能解释为什么 `submit()` 使用完美转发。

### 第三阶段：第 7--9 题

能够：

-   画出异步通信架构。
-   解释 IO ownership。
-   设计 sequence ID + pending map。
-   处理 timeout / late response。
-   理解 retry 与幂等性。
-   设计基本 reconnect 状态机。

### 第四阶段：第 10 题

再进入：

``` text
atomic
  ↓
CAS
  ↓
memory_order
  ↓
ABA
  ↓
memory reclamation
  ↓
lock-free queue
```

在前面没有掌握之前，不建议把"手撕无锁队列"作为主要秋招准备内容。
