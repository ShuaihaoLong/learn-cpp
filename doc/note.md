### thread
```cpp
void func();
std::thread thread1(func);
if(thread1.joinable()) {
    thread1.join();
}
// 线程分离
thread1.detach();
```
### 线程函数中数据未定义错误
1. 传递临时变量的问题
线程函数参数是引用变量时, 不能直接传入变量   
需要使用`std::ref()`, 将变量转为自身的引用变量
2. 传递指针或引用指向局部变量的问题
线程函数参数的生存周期必须包含线程执行时间
3. 传递指针或引用指向已释放的内存的问题
4. 类成员函数作为入口函数，类对象被提前释放
类成员函数作为入口函数，第一个入参必须是类对象实例或类指针（包括智能指针）
### 互斥量解决多线程资源共享问题
锁：`std::mutex`
```cpp
std::mutex mx;
mx.lock();
mx.unlock();
```
lock_guard和unique_lock
```cpp
std::mutex mx;
{
    std::lock_guard<std::mutex> lg(mx);
}
// lock_guard 不能拷贝和复制