# Linux高级环境编程作业
## 带缓存的文件操作类
- 实现一个带缓存的文件操作类；读写存入缓存，尽可能减少写盘和读盘
- 整个类只会操作一个文件
### 实现思路
- 构造Buffer类作为缓存
  - 在磁盘上读写，顺序读写更快，所以缓存数据使用数组而非链表（链表分配结点的空间并不连续）
  - 如果写缓存和读缓存在同一块缓存，需要对缓存中数据数组进行大量的移动操作，而且破坏了磁盘读写的连续性
  - 所以本文件操作类设置两个存数据的缓存，一个读缓存，一个写缓存；为了让读缓存更少地从磁盘中获取数据，每次读缓存都读出更多的数据；为管理这些数据，设置管理读数据元数据的缓存
### 具体结构
- CLFileOP类有三个缓存，一个读缓存，一个写缓存，一个管理读数据中元数据的缓存
  - 写缓存不需要信息，直接加入Buffer，Buffer满了直接写入磁盘
  - 读数据元数据缓存需要缓存元数据信息
  - 读缓存每次读磁盘读出更多数据
- 先写缓存，写缓存满了直接flush到磁盘上
- 读缓存前，先刷写缓存，读磁盘
- 读缓存满了后~~清除超过len的数据即可~~
  - 顺序写磁盘更能够节省时间，所以直接将整个缓存写入磁盘，重新加载读缓存
  - 重新加载读缓存的话，按照需要读取的长度二倍来读取，如果文件内容大于缓存大小的一半，只读缓存大小的一半
- 元数据缓存满了后，直接从头开始覆盖，不需要落盘
### 考虑加入
- 多线程读写缓存，加锁
