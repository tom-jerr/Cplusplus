#ifndef CLDISK_H_
#define CLDISK_H_

/* 磁盘最小的单位，可容纳512B数据，包含元数据 */
struct Block {
    int s_block_id;          // 块号
    int s_block_size;        // 块大小(固定为512B)
    int s_block_start;       // 块起始地址
    int s_block_end;         // 块结束地址
    bool s_block_isFree;     // 是否空闲
    char s_block_data[512];  // 数据
} __attribute__((aligned(4)));

/* 文件索引结点 */
struct Inode {
    int s_file_size;       // 文件大小
    char s_file_type[20];  // 文件类型
    int s_file_count;      // 文件的引用数
    char s_file_name[20];  // 文件名
    char* s_file_map[12];  // 文件物理块指针；后续可扩展到多级索引
} __attribute__((aligned(4)));

/* block位图 */
struct BlockBitmap {
    char s_block_bitmap[400];
} __attribute__((aligned(4)));

/* inode位图 */
struct InodeBitmap {
    char s_inode_bitmap[40];
} __attribute__((aligned(4)));

/* 索引节点表 */
struct InodeTable {
    Inode s_inode[100];
} __attribute__((aligned(4)));

/* 文件超级块 */
struct SuperBlock {
    int s_block_num;   // 磁盘块总数
    int s_block_size;  // 磁盘块大小
    int s_block_free;  // 空闲磁盘块数

    int s_inode_num;   // 索引节点总数
    int s_inode_size;  // 索引节点大小
    int s_inode_free;  // 空闲索引节点数
};

/* 磁盘 */
struct Disk {
    SuperBlock s_super_block;    // 超级块
    BlockBitmap s_block_bitmap;  // 磁盘块位图
    InodeBitmap s_inode_bitmap;  // 索引节点位图
    InodeTable s_inode_table;    // 索引节点表
    Block s_block[400];          // 磁盘块
};
#endif  // CLDISK_H_