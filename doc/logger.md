# 异步日志库
## LogStream
- LogBuffer作为缓冲区，可以极大地降低write系统调用的次数，提高性能
- 不使用C++自带的printf或stdio，目的为提高性能；同时stdio内部缓冲区不透明，可能会引起错误
- 使用Fmt并重载operator<<，可以格式化输出
### Benchmark测试
将1000000个字符LogStream输出、printf输出、stdio输出进行比较
~~~shell
lzy@lzydesktop:~/Workspace/TinyNetLib/build/test/logger$ /home/lzy/Workspace/TinyNetLib/build/test/logger/LOGGER_BENCHMARK
Running main() from /home/lzy/googletest/googletest/src/gtest_main.cc
[==========] Running 1 test from 1 test suite.
[----------] Global test environment set-up.
[----------] 1 test from BenchmarkTest
[ RUN      ] BenchmarkTest.LogStreamtest
------------------------ int ------------------------
benchPrintf 0.067910
benchStringStream 0.047903
benchLogStream 0.010158
------------------------ double  ------------------------
benchPrintf 0.331536
benchStringStream 0.363679
benchLogStream 0.007097
------------------------ int64_t ------------------------
benchPrintf 0.060488
benchStringStream 0.045848
benchLogStream 0.009716
------------------------ void* ------------------------
benchPrintf 0.060552
benchStringStream 0.052506
benchLogStream 0.007376
[       OK ] BenchmarkTest.LogStreamtest (1065 ms)
[----------] 1 test from BenchmarkTest (1065 ms total)

[----------] Global test environment tear-down
[==========] 1 test from 1 test suite ran. (1065 ms total)
[  PASSED  ] 1 test.
~~~