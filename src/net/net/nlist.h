#ifndef NLIST_H
#define NLIST_H


/**
 * @brief 链表结点类型
 * 采用的是双向循环链表，方便结点的删除。
 */
typedef struct _nlist_node_t {
  struct _nlist_node_t *next;  // 后继结点
  struct _nlist_node_t *pre;   // 前驱结点
} nlist_node_t;

/**
 * @brief 通用链表结构
 */
typedef struct _nlist_t {
  nlist_node_t head;  // 哨兵头结点
  int count;          // 结点数量
} nlist_t;


/**
 * @brief 头结点的初始化
 */
static inline void nlist_node_init(nlist_node_t *node) {
  node->pre = node->next = (nlist_node_t *)0;
}

/**
 * @brief 判断结点是否已挂载在链表上
 *
 * @param node
 * @return int
 */
static inline int nlist_is_mount(nlist_node_t *node) {
  return (node->next || node->pre);
}

/**
 * @brief 获取结点的后继结点
 */
static inline nlist_node_t *nlist_next(nlist_t *list, nlist_node_t *node) {
  return node->next == &list->head ? (nlist_node_t *)0 : node->next; // 遍历到链表的head节点，返回0
}

/**
 * @brief 获取结点的前一结点
 */
static inline nlist_node_t *nlist_pre(nlist_t *list, nlist_node_t *node) {
  return node->pre == &list->head ? (nlist_node_t *)0 : node->pre; // 遍历到链表的head节点，返回0
}

/**
 * 初始化链表
 * @param list 待初始化的链表
 */
static inline void nlist_init(nlist_t *list) {
  if (list == (nlist_t *)0) return;

  list->head.pre = list->head.next = &list->head;
  list->count = 0;
}

/**
 * 判断链表是否为空
 * @param list 判断的链表
 * @return 1 - 空，0 - 非空
 */
static inline int nlist_is_empty(nlist_t *list) { return list->count == 0; }

/**
 * 获取链表的结点数量
 * @param list 查询的链表
 * @return 结果的数据
 */
static inline int nlist_count(nlist_t *list) { return list->count; }

/**
 * 获取指定链表的第一个元素
 * @param list 查询的链表
 * @return 第一个元素
 */
static inline nlist_node_t *nlist_first(nlist_t *list) {
  return list->head.next == &list->head
             ? (nlist_node_t *)0
             : list->head.next;  // 如果链表为空，返回0
}

/**
 * 获取指定链接的最后一个元素
 * @param list 查询的链表
 * @return 最后一个元素
 */
static inline nlist_node_t *nlist_last(nlist_t *list) {
  return list->head.pre == &list->head ? (nlist_node_t *)0
                                       : list->head.pre;  // 如果链表为空，返回0
}

/**
 * @brief 将结构体中某个字段的地址转换为所在结构体的指针
 * 例如：
 * struct aa{
 *  .....
 *  int node;
 *  .....
 * };
 * struct aa a;
 * 1.求结点在所在结构中的偏移:定义一个指向0的指针，用(struct aa
 * *)&0->node，所得即为node字段在整个结构体的偏移
 */
#define noffset_in_parent(parent_type, node_name) \
  ((char *)&(((parent_type *)0)->node_name))

// 2.求node所在的结构体首址：node的地址 - node的偏移
// 即已知a->node的地址，求a的地址
#define noffset_to_parent(node, parent_type, node_name) \
  ((char *)node - noffset_in_parent(parent_type, node_name))

// 3. 进行转换: (struct aa *)addr
// 使用方式：net_node_to_parent(node_addr, struct aa, node)
#define nlist_entry(node, parent_type, node_name)                           \
  ((parent_type *)(node ? noffset_to_parent((node), parent_type, node_name) \
                        : 0))

#define nlist_for_each(node_ptr, list_ptr)                                \
  for (node_ptr = (list_ptr)->head.next; node_ptr != &((list_ptr)->head); \
       node_ptr = node_ptr->next)
#define nlist_for_each_safe(node_ptr, next_ptr, list_ptr)           \
  for (node_ptr = (list_ptr)->head.next, next_ptr = node_ptr->next; \
       node_ptr != &((list_ptr)->head);                             \
       node_ptr = next_ptr, next_ptr = node_ptr->next)

void nlist_insert_first(nlist_t *list, nlist_node_t *node);
void nlist_insert_last(nlist_t *list, nlist_node_t *node);
void nlist_insert_after(nlist_t *list, nlist_node_t *pre, nlist_node_t *node);
void nlist_insert_before(nlist_t *list, nlist_node_t *next, nlist_node_t *node);
nlist_node_t *nlist_remove(nlist_t *list, nlist_node_t *node);
nlist_node_t *nlist_remove_first(nlist_t *list);
nlist_node_t *nlist_remove_last(nlist_t *list);
void nlist_join(nlist_t *front, nlist_t *behind);

/**
 * @brief 转移链表资源所有权
 *
 * @param dest 目标链表, 必须为空链表
 * @param src 源链表, 转移后为空链表
 */
static inline void nlist_move(nlist_t *dest, nlist_t *src) {
  if (!nlist_is_empty(dest) || nlist_is_empty(src)) return;

  dest->head.next = src->head.next;
  src->head.next->pre = &dest->head;
  dest->head.pre = src->head.pre;
  src->head.pre->next = &dest->head;
  dest->count = src->count;

  nlist_init(src);
}

#endif /* NLIST_H */
