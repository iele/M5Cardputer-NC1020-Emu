#include "lru.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#ifdef ARDUINO
#include <Arduino.h>
#else
#endif

// 创建新的node节点
node_t *new_node(key_type key, value_type value) {
#ifdef ARDUINO
    node_t *node = (node_t *) malloc(sizeof(node_t));
#else
    node_t *node = (node_t *) malloc(sizeof(node_t));
#endif
    node->key = key;
    memcpy(node->value, value, sizeof(value_type));
    node->prev = NULL;
    node->next = NULL;
    return node;
}

// 删除node节点
void del_node(node_t *node) {
    free(node);
}

// 初始化哈希表
void init_hash_table(hash_table_t *ht, int size, unsigned int (*hash_func)(key_type key)) {
    ht->size = size;
#ifdef ARDUINO
    ht->list = (node_t **) malloc(size * sizeof(node_t *));
#else
    ht->list = (node_t **) malloc(size * sizeof(node_t *));
#endif
    memset(ht->list, 0x0, size * sizeof(node_t *));
    ht->hash_func = hash_func;
}

// 释放哈希表
void free_hash_table(hash_table_t *ht) {
    for (int i = 0; i < ht->size; i++) {
        node_t *node = ht->list[i];
        while (node) {
            node_t *next = node->next;
            del_node(node);
            node = next;
        }
    }
    free(ht->list);
}

// 重置哈希表为空
void empty_hash_table(hash_table_t *ht) {
    for (int i = 0; i < ht->size; i++) {
        node_t *node = ht->list[i];
        while (node) {
            node_t *next = node->next;
            del_node(node);
            node = next;
        }
        ht->list[i] = NULL;
    }
}

// 查找哈希表中是否存在key为key的节点，并将该节点移动到链表头部
node_t *find_node(hash_table_t *ht, key_type key) {
    unsigned int hash = ht->hash_func(key) % ht->size;
    node_t *node = ht->list[hash];
    while (node) {
        if (node->key == key) {
            // 将该节点移动到链表头部
            if (node->prev) {
                node->prev->next = node->next;
                if (node->next) {
                    node->next->prev = node->prev;
                }
                node->prev = NULL;
                node->next = ht->list[hash];
                ht->list[hash]->prev = node;
                ht->list[hash] = node;
            }
            return node;
        }
        node = node->next;
    }
    return NULL;
}

// 插入键值对
void insert_value(hash_table_t *ht, key_type key, value_type value) {
    // 先查找哈希表中是否存在key
    node_t *node = find_node(ht, key);
    if (node) {
        // 如果存在，则更新value
        memcpy(node->value, value, sizeof(value_type));
    } else {
        // 如果不存在，则插入新节点
        node = new_node(key, value);
        unsigned int hash = ht->hash_func(key) % ht->size;
        // 如果链表不为空，则将新节点插入链表头部
        if (ht->list[hash]) {
            ht->list[hash]->prev = node;
            node->next = ht->list[hash];
        }
        ht->list[hash] = node;
    }
}

// 删除键值对
void delete_value(hash_table_t *ht, key_type key) {
    // 先查找哈希表中是否存在key
    node_t *node = find_node(ht, key);
    if (node) {
        // 如果存在，则将该节点从链表中删除
        unsigned int hash = ht->hash_func(key) % ht->size;
        if (node->prev) {
            node->prev->next = node->next;
        } else {
            ht->list[hash] = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
        del_node(node);
    }
}

// 清空哈希表
void clear_hash_table(hash_table_t *ht) {
    empty_hash_table(ht);
}

// 打印哈希表中的键值对
void print_hash_table(hash_table_t *ht) {
    for (int i = 0; i < ht->size; i++) {
        node_t *node = ht->list[i];
        printf("[%d]: ", i);
        while (node) {
            printf("(%d, %02X%02X%02X...), ", node->key, node->value[0], node->value[1],
                   node->value[2]);
            node = node->next;
        }
        printf("\n");
    }
}

// 哈希函数
unsigned int hash_func(key_type key) {
    return (unsigned int) key;
}


// 初始化LRU结构体
void init_lru(lru_t *lru, int capacity) {
    init_hash_table(&lru->ht, 4, hash_func); // 初始化哈希表大小为100
    lru->head = NULL;
    lru->tail = NULL;
    lru->capacity = capacity;
    lru->size = 0;
}

// 释放LRU结构体
void free_lru(lru_t *lru) {
    free_hash_table(&lru->ht);
}

// 重置LRU为空
void empty_lru(lru_t *lru) {
    clear_hash_table(&lru->ht);
    lru->head = NULL;
    lru->tail = NULL;
    lru->size = 0;
}

// 将节点插入链表头部
void insert_node_to_head(lru_t *lru, node_t *node) {
    if (lru->head) {
        node->next = lru->head;
        lru->head->prev = node;
        lru->head = node;
    } else {
        lru->head = node;
        lru->tail = node;
    }
}

// 移动节点到链表头部
void move_node_to_head(lru_t *lru, node_t *node) {
    if (node != lru->head) {
        if (node == lru->tail) {
            lru->tail = node->prev;
        }
        if (node->prev) {
            node->prev->next = node->next;
        }
        if (node->next) {
            node->next->prev = node->prev;
        }
        node->prev = NULL;
        node->next = lru->head;
        lru->head->prev = node;
        lru->head = node;
    }
}

// 从链表尾部删除节点
node_t *delete_node_from_tail(lru_t *lru) {
    if (lru->tail) {
        node_t *node = lru->tail;
        lru->tail = node->prev;
        if (lru->tail) {
            lru->tail->next = NULL;
        } else {
            lru->head = NULL;
        }
        node->prev = NULL;
        node->next = NULL;
        return node;
    }
    return NULL;
}

// 获取value
bool get_value(lru_t *lru, key_type key, value_type** value) {
    node_t *node = find_node(&lru->ht, key);
    if (node) {
        *value = &(node->value);
        // memcpy(value, node->value, sizeof(value_type));
        move_node_to_head(lru, node);
        return true;
    }
    return false;
}

// 插入键值对
void insert_value_to_lru(lru_t *lru, key_type key, value_type value) {
    node_t *node = find_node(&lru->ht, key);
    if (node) {
        // 如果节点已存在，则更新value并移动到链表头部
        memcpy(node->value, value, sizeof(value_type));
        move_node_to_head(lru, node);
    } else {
        // 如果节点不存在，则插入新节点并将其移动到链表头部
        if (lru->size >= lru->capacity) {
            // 如果LRU已满，则先删除链表尾部节点，再删除哈希表中该节点对应的键值对
            node_t *tail_node = delete_node_from_tail(lru);
            delete_value(&lru->ht, tail_node->key);
            del_node(tail_node);
            lru->size--;
        }
        node = new_node(key, value);
        insert_node_to_head(lru, node);
        insert_value(&lru->ht, key, value);
        lru->size++;
    }
}

// 删除键值对
void delete_value_from_lru(lru_t *lru, key_type key) {
    node_t *node = find_node(&lru->ht, key);
    if (node) {
        delete_node_from_tail(lru);
        delete_value(&lru->ht, key);
        del_node(node);
        lru->size--;
    }
}

// 清空LRU
void clear_lru(lru_t *lru) {
    empty_lru(lru);
}

// 打印LRU中的键值对
void print_lru(lru_t *lru) {
    printf("LRU (capacity=%d, size=%d):\n", lru->capacity, lru->size);
    printf("  chain: ");
    node_t *node = lru->head;
    while (node) {
        printf("%d,", node->key);
        node = node->next;
    }
    printf("\n");
    print_hash_table(&lru->ht);
}

int main_test() {
    lru_t lru;
    init_lru(&lru, 3);

    uint8_t v1[] = {0x11, 0x22, 0x33, 0x44};
    uint8_t v2[] = {0x55, 0x66, 0x77, 0x88};
    uint8_t v3[] = {0xaa, 0xbb, 0xcc, 0xdd};
    uint8_t v4[] = {0xee, 0xff, 0x00, 0x11};
    uint8_t value[0x8000];

    insert_value_to_lru(&lru, 1, v1);
    insert_value_to_lru(&lru, 2, v2);
    insert_value_to_lru(&lru, 3, v3);
    print_lru(&lru); // LRU (capacity=3, size=3): chain: 3,2,1
    // get_value(&lru, 2, value);
    printf("get(2)=%02X%02X%02X%02X\n", value[0], value[1], value[2], value[3]); // get(2)=55667788
    insert_value_to_lru(&lru, 4, v4);
    print_lru(&lru); // LRU (capacity=3, size=3): chain: 4,2,3

    free_lru(&lru);
    return 0;
}
