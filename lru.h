
#ifndef LRU_H_
#define LRU_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

// 定义key和value的类型
typedef int key_type;
typedef uint8_t value_type[0x200];

// 定义节点结构体，包含key、value和指向上一个和下一个节点的指针
typedef struct node_t {
    key_type key;
    value_type value;
    struct node_t *prev;
    struct node_t *next;
} node_t;

// 定义哈希表结构体，包含哈希表大小、哈希表数组和哈希函数
typedef struct hash_table_t {
    int size;
    node_t **list;

    unsigned int (*hash_func)(key_type key);
} hash_table_t;

// 创建新的node节点
node_t *new_node(key_type key, value_type value);

// 删除node节点
void del_node(node_t *node);

// 初始化哈希表
void init_hash_table(hash_table_t *ht, int size, unsigned int (*hash_func)(key_type key));


// 释放哈希表
void free_hash_table(hash_table_t *ht);

// 重置哈希表为空
void empty_hash_table(hash_table_t *ht);

// 查找哈希表中是否存在key为key的节点，并将该节点移动到链表头部
node_t *find_node(hash_table_t *ht, key_type key);

// 插入键值对
void insert_value(hash_table_t *ht, key_type key, value_type value);

// 删除键值对
void delete_value(hash_table_t *ht, key_type key);


// 清空哈希表
void clear_hash_table(hash_table_t *ht);

// 打印哈希表中的键值对
void print_hash_table(hash_table_t *ht);

// 哈希函数
unsigned int hash_func(key_type key);

// LRU结构体，包含哈希表、链表头指针和链表尾指针
typedef struct lru_t {
    hash_table_t ht;
    node_t *head;
    node_t *tail;
    int capacity;
    int size;
} lru_t;

// 初始化LRU结构体
void init_lru(lru_t *lru, int capacity);

// 释放LRU结构体
void free_lru(lru_t *lru);

// 重置LRU为空
void empty_lru(lru_t *lru);

// 将节点插入链表头部
void insert_node_to_head(lru_t *lru, node_t *node);

// 移动节点到链表头部
void move_node_to_head(lru_t *lru, node_t *node);

// 从链表尾部删除节点
node_t *delete_node_from_tail(lru_t *lru);

// 获取value
bool get_value(lru_t *lru, key_type key, value_type** value);

// 插入键值对
void insert_value_to_lru(lru_t *lru, key_type key, value_type value);

// 删除键值对
void delete_value_from_lru(lru_t *lru, key_type key);

// 清空LRU
void clear_lru(lru_t *lru);

// 打印LRU中的键值对
void print_lru(lru_t *lru);

#endif /* LRU_H_ */
