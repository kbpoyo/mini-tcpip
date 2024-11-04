#include "nlist.h"

/**
 * 将Node插入指定结点之后
 * @param list 操作的链表
 * @param pre 前一结点
 * @param node 待插入的结点
 */
void nlist_insert_after(nlist_t *list, nlist_node_t *pre, nlist_node_t *node) {
  if (list == (nlist_t *)0 || pre == (nlist_node_t *)0 ||
      node == (nlist_node_t *)0) {
    return;
  }

  // 调整node
  node->pre = pre;
  node->next = pre->next;

  // 调整pre的后继
  pre->next->pre = node;
  pre->next = node;

  list->count++;
}

/**
 * @brief 将node插入到next之前
 *
 * @param list
 * @param next
 * @param node
 */
void nlist_insert_before(nlist_t *list, nlist_node_t *next,
                         nlist_node_t *node) {
  if (list == (nlist_t *)0 || next == (nlist_node_t *)0 ||
      node == (nlist_node_t *)0) {
    return;
  }

  // 调整node
  node->next = next;
  node->pre = next->pre;

  // 调整next的前驱
  next->pre->next = node;
  next->pre = node;

  list->count++;
}

/**
 * 将指定表项插入到指定链表的头部
 * @param list 待插入的链表
 * @param node 待插入的结点
 */
void nlist_insert_first(nlist_t *list, nlist_node_t *node) {
  nlist_insert_after(list, &list->head, node);
}
/**
 * 将指定表项插入到指定链表的尾部
 * @param list 操作的链表
 * @param node 待插入的结点
 */
void nlist_insert_last(nlist_t *list, nlist_node_t *node) {
  nlist_insert_before(list, &list->head, node);
}

/**
 * @brief 移除链表首个结点
 */
nlist_node_t *nlist_remove_first(nlist_t *list) {
  nlist_node_t *first = nlist_first(list);
  if (first) {
    nlist_remove(list, first);
  }
  return first;
}

/**
 * @brief 移除链表末尾结点
 */
nlist_node_t *nlist_remove_last(nlist_t *list) {
  nlist_node_t *last = nlist_last(list);
  if (last) {
    nlist_remove(list, last);
  }
  return last;
}

/**
 * 移除指定链表的中的表项
 * 不检查node是否在结点中
 */
nlist_node_t *nlist_remove(nlist_t *list, nlist_node_t *remove_node) {
  if (list == (nlist_t *)0 || remove_node == (nlist_node_t *)0 || &list->head == remove_node) {
    return (nlist_node_t *)0;
  }

  // 如果有前，则调整前的后继
  if (remove_node->pre) {
    remove_node->pre->next = remove_node->next;
  }

  // 如果有后，则调整后的前驱
  if (remove_node->next) {
    remove_node->next->pre = remove_node->pre;
  }

  // 清空node指向
  remove_node->pre = remove_node->next = (nlist_node_t *)0;
  list->count--;
  return remove_node;
}

/**
 * @brief 合并两个链表, 合并成功后，behind链表将清空
 *
 * @param front 放在前面的链表, 合并后会被修改
 * @param behind 放在后面的链表, 合并后会被清空
 */
void nlist_join(nlist_t *front, nlist_t *behind) {
  if (front == (nlist_t *)0 || behind == (nlist_t *)0) {
    return;
  }

  // 如果behind为空，则直接返回
  if (behind->count == 0) {
    return;
  }

  // 如果front为空，则直接将behind移动到front
  if (front->count == 0) {
    nlist_move(front, behind);
    return;
  }

  // 将behind和front首尾相连
  behind->head.next->pre = front->head.pre; // 调整behind的头结点前驱为front的尾结点
  front->head.pre->next = behind->head.next; // 调整front的尾结点后继为behind的头结点
  // 调整front指向behind的尾结点
  front->head.pre = behind->head.pre; 
  behind->head.pre->next = &front->head; 
  // 调整front的结点数量
  front->count += behind->count;

  // 清空behind
  nlist_init(behind);
}
