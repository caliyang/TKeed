//
// Latest edit by TeeKee on 2017/7/23.
//

#ifndef TK_LIST_H
#define TK_LIST_H

/*为什么需要这一选择编译过程，01？？？*/
#ifndef NULL
#define NULL 0
#endif

/*list_head_t，01？？？*/
/*双向链表*/
typedef struct list_head {
    struct list_head *prev, *next;
}list_head;

// 初始化链表
/*步骤包括：创建哨兵节点并在哨兵间互联，本例中只有一个哨兵，因此自己和自己互联*/
/*结尾的反斜杠起到换行作用，用于宏定义和字符串换行，其中宏定义中使用居多；
    如果一行代码有很多元素，导致太长影响阅读，可以通过在结尾加\的方式，实现换行，
    并且编译时会忽略\及其后的换行符，当做一行处理。*/
#define INIT_LIST_HEAD(ptr) do {\
    struct list_head *_ptr = (struct list_head *)ptr;   \
    (_ptr)->next = (_ptr); (_ptr->prev) = (_ptr);       \
}while(0)

/* 计算member成员的在结构体内偏移值，结构体的地址 + 偏移值 = 结构体某成员的地址 */
/* 成员函数能这样操作吗，02？？？ */
/* 具体解释：
     (TYPE*)0，将0强转为TYPE类型的指针,且指向了0地址空间；
     (TYPE*)0->MEMEBER，表示结构体中的某个成员；
     &((TYPE*)0->MEMBER)，获取成员在结构体的位置,因为起始为0,所以获取的地址即为实际的偏移地址； */
/* 注意事项：
     如果利用NULL指针来访问type的成员当然是非法的，但typeof( ((type *)0)->member )是想取该成员的类型，
     所以编译器不会生成访问type成员的代码，类似的代码&( ((type *)0)->member )在最前面有个取地址符&，
     它的意图是想取member的地址，所以编译器同样会优化为直接取地址。 */
/* 该宏有定义在kernel.h中 */
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

/* 计算包含ptr指向的变量所在的type类型结构的地址 */
/* ptr是指向正被使用的某类型变量指针；
   type是包含ptr指向的变量类型的结构类型；
   member是type结构体中的成员，类型与ptr指向的变量类型一样。*/
/* 具体解释：
     通过typeof关键字定义一个与type结构体的member成员相同的类型的变量__mptr且将ptr值赋给它；
     用宏offsetof(type,member),获取member成员在type结构中的偏移量（原型：offsetof(TYPE,MEMBER) ((size_t)&(TYPE *)0)->MEMBER). 定义在stddef.h.）
     最后将__mptr值减去这个偏移量，就得到这个结构变量的地址了（亦指针）。 */
/* 该宏有定义在kernel.h中 */
/* 为什么__mptr前面有“__”，01？？？ */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );     \
})

#define list_entry(ptr, type, member) \
    container_of(ptr, type, member)

/* 功能是从首节点向后遍历到末节点 */
/* 该链表是头尾相连的双向链表，即头哨兵和尾哨兵由同一个哨兵充当 */
#define list_for_each(pos, head) \
    for (pos = (head)->next; pos != (head); pos = pos->next)

/*功能是从末节点向前遍历到首节点；*/
#define list_for_each_prev(pos, head) \
    for (pos = (head)->prev; pos != (head); pos = pos->prev)

// 插入新节点
/* 插入新节点时，有一个原则就是不能将原始链表弄丢，
     所以在断开原始连接之前，我们应该先要将即将断开部分的前后连接保存下来 */
/* 为什么函数名的前面有“__”，01？？？ */
static inline void __list_add(struct list_head *_new, struct list_head *prev, struct list_head *next) {
    _new->next = next;
    next->prev = _new;
    prev->next = _new;
    _new->prev = prev;
}

// 头部新增
static inline void list_add(struct list_head *_new, struct list_head *head) {
    __list_add(_new, head, head->next);
}

// 尾部新增
static inline void list_add_tail(struct list_head *_new, struct list_head *head) {
    __list_add(_new, head->prev, head);
}

// 删除节点
/*删除的是位于prev和next之间的节点*/
/*为什么函数名的前面有“__”，01？？？*/
static inline void __list_del(struct list_head *prev, struct list_head *next) {
    prev->next = next;
    next->prev = prev;
}

// 删除entry节点
/*entry节点是啥，01？？？*/
static inline void list_del(struct list_head *entry) {
    __list_del(entry->prev, entry->next);
}

// 链表判空
/*只有哨兵节点意味着链表为空*/
static inline int list_empty(struct list_head *head) {
    return (head->next == head) && (head->prev == head);
}

#endif 
