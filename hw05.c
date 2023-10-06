#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

struct bplus_tree
{
    int *keys;
    struct bplus_tree **children;
    int key_cnt;
    bool is_leaf;
    int *val;
    struct bplus_tree *prev;
    struct bplus_tree *next;
};

//  初始化B+ Tree的node
struct bplus_tree *initialize(int value, int order, bool isLeaf)
{
    struct bplus_tree *new_node = malloc(sizeof(struct bplus_tree));
    new_node->keys = malloc(sizeof(int *) * order * 2);
    for (int i = 0; i < 2 * order; i++)
        new_node->keys[i] = 0;

    new_node->children = malloc(sizeof(struct bplus_tree *) * (2 * order + 1));
    for (int i = 0; i < 2 * order + 1; i++)
        new_node->children[i] = NULL;

    new_node->val = malloc(sizeof(int) * order * 2);
    for (int i = 0; i < 2 * order; i++)
        new_node->val[i] = 0;

    if (isLeaf)
        new_node->val[0] = value;
    else
        new_node->keys[0] = value;

    new_node->key_cnt = 0;
    new_node->is_leaf = isLeaf;
    new_node->prev = NULL;
    new_node->next = NULL;
    return new_node;
}

int count_value_in_leaf_node(struct bplus_tree *node, int capacity)
{
    int cnt = 0;
    for (int i = 0; i < capacity; i++)
    {
        if (node->val[i] != 0)
            cnt++;
    }
    return cnt;
}

//  compute the least amount of values in each node according to order
int compute_threshold(int order)
{
    if (order % 2 != 0)
        return order / 2 + 1;
    return order / 2;
}

int find_insert_index_of_leaf_node(struct bplus_tree *node, int value, int order)
{
    struct bplus_tree *ptr = node;
    int i = 0;
    int cnt = count_value_in_leaf_node(ptr, 2 * order);
    for (i = 0; i < cnt; i++)
    {
        if (ptr->val[i] > value)
            return i;
    }
    // printf("insert index: %d\n", i);
    return i;
}

int find_insert_index_of_internal_node(struct bplus_tree *node, int key, int order)
{
    struct bplus_tree *ptr = node;
    int i = 0;
    for (i = 0; i < 2 * order; i++)
    {
        if (ptr->keys[i] > key)
            return i;
        if (ptr->keys[i] == 0)
            return i;
    }
    return i;
}

void insert_in_leaf_node(struct bplus_tree *node, int value, int order)
{
    int idx = find_insert_index_of_leaf_node(node, value, order);
    int cnt = count_value_in_leaf_node(node, 2 * order);
    //  把從idx之後的值全部往後移一個index
    for (int i = cnt; i > idx; i--)
        node->val[i] = node->val[i - 1];
    node->val[idx] = value;
    return;
}

bool leaf_is_full(struct bplus_tree *node, int capacity)
{
    int cnt = count_value_in_leaf_node(node, capacity);
    if (cnt == capacity)
        return true;
    return false;
}

bool parent_is_full(struct bplus_tree *node, int order)
{
    if (node->key_cnt == 2 * order)
        return true;
    return false;
}

//  qsort compare function --> it is used to sort the values or keys in split() function
int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int count_key_in_node(struct bplus_tree *node, int order)
{
    int cnt = 0;
    for (int i = 0; i < 2 * order; i++)
    {
        if (node->keys[i] != 0)
            cnt++;
    }
    return cnt;
}

bool node_is_full(struct bplus_tree *node, int order)
{
    int cnt = count_key_in_node(node, order);
    printf("%d keys in node\n", cnt);
    if (cnt == 2 * order)
        return true;
    return false;
}

struct bplus_tree *find_parent_node(struct bplus_tree *root, struct bplus_tree *current, int order)
{
    if (root == current)
        return NULL;

    struct bplus_tree *ptr = root;
    int key = current->keys[0];
    if (current->is_leaf)
        key = current->val[0];
    // printf("key in find_parent_node: %d\n", key);
    while (ptr)
    {
        for (int i = 0; i < 2 * order - 1; i++)
        {
            // if (ptr->keys[i] == 0 && key >= ptr->keys[i - 1])
            // {
            //     ptr = ptr->children[i];
            //     break;
            // }

            if (ptr->keys[i] > key)
            {
                if (ptr->children[i] == current)
                    return ptr;
                ptr = ptr->children[i];
                break;
            }
        }
        if (key >= ptr->keys[2 * order - 2])
        {
            // printf("here\n");
            // printf("ptr->children[order - 1]->val[0]: %d\n", ptr->children[order - 1]->val[0]);
            if (ptr->children[2 * order - 1] == current)
            {
                // printf("found\n");
                return ptr;
            }

            ptr = ptr->children[2 * order - 1];
        }
    }
    return ptr;
}

//  調整internal node, root為B+ tree的root, node為value要insert回去的那個node, new_node為新建立且尚未接上的新node, value為要insert到node的value, order為B+ tree的order
struct bplus_tree *split_internal_node(struct bplus_tree *root, struct bplus_tree *node, struct bplus_tree *new_node, int value, int order)
{
    //  root目前為leaf node 而且滿了 需要建立一個新的root並且連接兩個leaf node
    if (node == NULL && root->is_leaf)
    {
        // printf("node is NULL\n");
        struct bplus_tree *new_root = malloc(sizeof(struct bplus_tree));
        new_root->keys = malloc(sizeof(int) * (order - 1));
        new_root->keys[0] = value;
        new_root->children = malloc(sizeof(struct bplus_tree) * order);
        new_root->children[0] = root;
        new_root->children[1] = new_node;
        return new_root;
    }
    //  node目前不是leaf node 可以檢查看看是否有空間可以insert value
    if (node == NULL)
    {
        // root滿了 要split
        if (node_is_full(root, order))
        {
            //  將key存到values中 用qsort排序 再拆成兩部分
            int *values = malloc(sizeof(int) * (2 * order + 1));
            for (int i = 0; i < 2 * order; i++)
                values[i] = root->keys[i];
            values[2 * order] = value;
            qsort(values, 2 * order, sizeof(int), cmpfunc);

            //  前半部分保留在原父節點
            int threshold = compute_threshold(2 * order - 1);
            for (int i = 0; i < threshold; i++)
                root->keys[i] = values[i];
            for (int i = threshold; i < 2 * order; i++)
                root->keys[i] = 0;

            //  後半部分移至新建的父節點
            struct bplus_tree *new_parent_node = malloc(sizeof(struct bplus_tree));
            new_parent_node->keys = malloc(sizeof(int) * (2 * order));
            new_parent_node->is_leaf = false;
            new_parent_node->children = malloc(sizeof(struct bplus_tree *) * 2 * order);

            for (int i = threshold; i < 2 * order; i++)
                new_parent_node->keys[i - threshold] = values[i];

            //  調整pointers
            if (new_node != NULL)
            {
                int new_key_cnt = count_key_in_node(root, order);
                int i = 0;
                for (i = 0; i < 2 * order - new_key_cnt; i++)
                {
                    new_parent_node->children[i + 1] = root->children[new_key_cnt + i + 1];
                }
                new_parent_node->children[i + 1] = new_node;
                for (int i = new_key_cnt + 1; i < 2 * order + 1; i++)
                    root->children[i] = NULL;
            }

            //  insert回parent node
            struct bplus_tree *parent = find_parent_node(root, root, order);
            return split_internal_node(root, parent, new_parent_node, new_parent_node->keys[0], order);
        }
        //  root沒滿 直接insert到root
        else
        {
            //  找到key的insert index
            int idx = find_insert_index_of_internal_node(root, value, order);

            //  將key[idx]之後的都往右一格
            for (int i = 2 * order - 2; i > idx; i--)
                root->keys[i] = root->keys[i - 1];

            //  insert key
            root->keys[idx] = value;

            //  將children[idx + 1]都往右一格
            for (int i = 2 * order - 1; i > idx + 1; i--)
                root->children[i] = root->children[i - 1];

            //  children[idx + 1] = new_node
            root->children[idx + 1] = new_node;

            //  return root
            return root;
        }
    }

    if (node_is_full(node, order))
    {
        //  將key存到values中 用qsort排序 再拆成兩部分
        int *values = malloc(sizeof(int) * (2 * order + 1));
        for (int i = 0; i < 2 * order; i++)
            values[i] = node->keys[i];
        values[2 * order] = value;
        qsort(values, 2 * order, sizeof(int), cmpfunc);

        //  前半部分保留在原父節點
        int threshold = compute_threshold(2 * order - 1);
        for (int i = 0; i < threshold; i++)
            node->keys[i] = values[i];
        for (int i = threshold; i < 2 * order; i++)
            node->keys[i] = 0;

        //  後半部分移至新建的父節點
        struct bplus_tree *new_parent_node = malloc(sizeof(struct bplus_tree));
        new_parent_node->keys = malloc(sizeof(int) * (2 * order));
        new_parent_node->is_leaf = false;
        new_parent_node->children = malloc(sizeof(struct bplus_tree *) * 2 * order);

        for (int i = threshold; i < 2 * order; i++)
            new_parent_node->keys[i - threshold] = values[i];

        //  調整pointers
        if (new_node != NULL)
        {
            int new_key_cnt = count_key_in_node(node, order);
            int i = 0;
            for (i = 0; i < 2 * order - new_key_cnt; i++)
            {
                new_parent_node->children[i + 1] = node->children[new_key_cnt + i + 1];
            }
            new_parent_node->children[i + 1] = new_node;
            for (int i = new_key_cnt + 1; i < 2 * order + 1; i++)
                node->children[i] = NULL;
        }

        //  insert回parent node
        struct bplus_tree *parent = find_parent_node(root, node, order);
        return split_internal_node(root, parent, new_parent_node, new_parent_node->keys[0], order);
        // return root;
    }
    else
    {
        //  找到key的insert index
        int idx = find_insert_index_of_internal_node(node, value, order);

        //  將key[idx]之後的都往右一格
        for (int i = order - 2; i > idx; i--)
            node->keys[i] = node->keys[i - 1];

        //  insert key
        node->keys[idx] = value;

        //  將children[idx + 1]都往右一格
        for (int i = order - 1; i > idx + 1; i--)
            node->children[i] = node->children[i - 1];

        //  children[idx + 1] = new_node
        node->children[idx + 1] = new_node;

        //  return root
        return root;
    }
}

//  當leaf node在insert的時候出現overflow --> 會需要將資料split into two parts
struct bplus_tree *split(struct bplus_tree *node, int value, int order)
{

    if (!node->is_leaf)
    {
        split_internal_node(node, node, NULL, value, order);
    }

    //  先把values依序放在一個array中
    int *values = malloc(sizeof(int) * order * 2);
    int idx = find_insert_index_of_leaf_node(node, value, order);
    int i = 0;
    for (i = 0; i < idx; i++)
        values[i] = node->val[i];
    values[idx] = value;
    for (i = idx; i < order; i++)
        values[i + 1] = node->val[i];

    //  將前半部存到ptr中
    int threshold = compute_threshold(order);
    for (int i = 0; i < threshold; i++)
        node->val[i] = values[i];
    for (int i = threshold; i < order; i++)
    {
        // free(ptr->val[i]);
        node->val[i] = '_';
    }
    // free(ptr->val[threshold + 1]);

    //  將後半部存到新的node中
    struct bplus_tree *new_node = malloc(sizeof(struct bplus_tree));
    new_node->val = malloc(sizeof(int) * order * 2);
    for (int i = threshold; i < order + 1; i++)
    {
        // new_node->val[i - threshold] = malloc(sizeof(int));
        new_node->val[i - threshold] = values[i];
    }
    //  tail接上原本的linked list
    new_node->next = node->next; //  new_node->next接ptr->next
    new_node->prev = node;       //  new_node->prev接ptr
    if (node->next != NULL)
        node->next->prev = new_node; //  當ptr->next不是null的時候 ptr->next的prev接new_node
    node->next = new_node;
    return node;
}

//  建立第一個internal node
struct bplus_tree *create_new_root(struct bplus_tree *left_node, struct bplus_tree *right_node, int order)
{
    struct bplus_tree *new_root = initialize(right_node->val[0], order, false);
    new_root->children[0] = left_node;
    new_root->children[1] = right_node;
    return new_root;
}

struct bplus_tree *find_leaf_node(struct bplus_tree *root, int value, int order)
{
    struct bplus_tree *ptr = root;
    struct bplus_tree *prev = NULL;
    int i = 0;
    while (ptr && !ptr->is_leaf)
    {
        for (i = 0; i < order - 1; i++)
        {
            //  key沒有值
            if (ptr->keys[i] == 0)
            {
                prev = ptr;
                ptr = ptr->children[i];
                break;
            }
            //  繼續找下一個internal node
            if (value < ptr->keys[i])
            {
                prev = ptr;
                ptr = ptr->children[i];
                break;
            }
        }
    }

    if (ptr == NULL)
        return prev;
    return ptr;
}

int find_key_index(struct bplus_tree *node, int key, int order)
{
    int i = 0;
    for (i = 0; i < order - 1; i++)
    {
        if (node->keys[i] == 0)
            return i;
        if (key < node->keys[i])
            return i;
    }
    return i;
}

void Display(struct bplus_tree *root, int order, int tabCnt)
{
    if (root != NULL)
    {
        for (int i = 0; i < tabCnt; i++)
            printf("\t");
        // printf("\n");
        if (!root->is_leaf)
        {
            printf("(");
            for (int i = 0; i < order * 2; i++)
            {
                if (root->keys[i] != 0)
                    printf("%d", root->keys[i]);
                else
                    printf("_");
                if (i < order * 2 - 1)
                    printf(",");
            }
            printf(")\n");
        }
        else
        {
            printf("[");
            for (int i = 0; i < 2 * order; i++)
            {
                if (root->val[i] != 0)
                    printf("%d", root->val[i]);
                else
                    printf("_");
                if (i < 2 * order - 1)
                    printf(",");
            }
            printf("]\n");
        }

        for (int i = 0; i < 2 * order + 1; i++)
            Display(root->children[i], order, tabCnt + 1);
    }
}

//  根據value 找到對應insert的leaf node
struct bplus_tree *find_insert_node(struct bplus_tree *root, int value, int order)
{
    if (root->is_leaf)
        return root;

    struct bplus_tree *ptr = root;
    while (!ptr->is_leaf)
    {
        // printf("find_insert_node\n");
        for (int i = 0; i < 2 * order; i++)
        {
            if (ptr->keys[i] == 0 && value >= ptr->keys[i - 1])
            {
                // printf("go to next node\n");
                ptr = ptr->children[i];
                // printf("ptr->key[0]: %d\n", ptr->keys[0]);
                break;
            }

            // printf("ptr->keys[%d]: %d\n", i, ptr->keys[i]);
            if (ptr->keys[i] > value)
            {
                // printf("go\n");
                ptr = ptr->children[i];
                break;
            }
        }
        // printf("ptr->keys[order - 2]: %d\n", ptr->keys[order - 2]);
        if (!ptr->is_leaf && ptr->keys[2 * order - 2] <= value && ptr->keys[2 * order - 2] != 0)
        {
            // printf("been here before\n");
            ptr = ptr->children[order - 1];
            // printf("go 2.0\n");
            // printf("ptr->val[0]: %d\n", ptr->val[0]);
        }
    }
    return ptr;
}

struct bplus_tree *balance(struct bplus_tree *node, int value, int order)
{
    //  先把node->val跟要insert的value存在values內
    int *values = malloc(sizeof(int) * order * 2 + 1);
    for (int i = 0; i < order * 2; i++)
        values[i] = node->val[i];
    values[order * 2] = value;

    // for (int i = 0; i < order * 2 + 1; i++)
    //     printf("values[%d]: %d\n", i, values[i]);

    //  使用qsort對values排序(由小到大)
    qsort(values, 2 * order + 1, sizeof(int), cmpfunc);

    //  將values前半部存在node中
    int threshold = compute_threshold(order * 2);
    for (int i = 0; i < threshold; i++)
        node->val[i] = values[i];
    for (int i = threshold; i < order * 2; i++)
        node->val[i] = 0;

    //  後半部存在 new_node中
    struct bplus_tree *new_node = initialize(0, order, true);
    for (int i = threshold; i < order * 2 + 1; i++)
    {
        new_node->val[i - threshold] = values[i];
        // printf("new_node->val[%d]: %d\n", i - threshold, values[i]);
    }

    //  接上linked list
    new_node->next = node->next;
    if (node->next)
        node->next->prev = new_node;
    node->next = new_node;
    new_node->prev = node;

    //  return new_node;
    return new_node;
}

struct bplus_tree *insert(struct bplus_tree *root, int insert_value, int order)
{
    //  沒有節點 直接回傳新的節點
    if (root == NULL)
    {
        return initialize(insert_value, order, true);
    }

    //  找到node
    struct bplus_tree *target_node = find_insert_node(root, insert_value, order);

    //  找出插入index
    int idx = find_insert_index_of_leaf_node(target_node, insert_value, order);

    //  判斷是否還有空間
    if (leaf_is_full(target_node, 2 * order))
    {
        //  沒空間 建立新的node
        struct bplus_tree *new_node = balance(target_node, insert_value, order);

        //  return split_internal_node(root, target_node, NULL, new_node->val[0], order)之類的
        target_node = find_parent_node(root, target_node, order);
        return split_internal_node(root, target_node, new_node, new_node->val[0], order);
    }
    else
    {
        //  有空間 insert value at index
        for (int i = 2 * order - 1; i > idx; i--)
            target_node->val[i] = target_node->val[i - 1];
        target_node->val[idx] = insert_value;
        return root;
    }
}

//  把struct bplus_tree中malloc過的變數都free一次
void free_bplus_tree_node(struct bplus_tree *node)
{
    free(node->children);
    free(node->keys);
    free(node->val);
    free(node);
}

//  把leaf node free掉
void free_leaf_node(struct bplus_tree *root, int order)
{
    //  找到left most的leaf node
    struct bplus_tree *ptr = root;
    while (!ptr->is_leaf)
    {
        for (int i = 0; i < order; i++)
        {
            if (ptr->children[i] != NULL)
            {
                ptr = ptr->children[i];
                break;
            }
        }
    }
    // printf("linked list head val: %d\n", ptr->val[0]);
    while (ptr)
    {
        // printf("ptr->val[0]: %d\n", ptr->val[0]);
        struct bplus_tree *prev = ptr;
        ptr = ptr->next;
        free_bplus_tree_node(prev);
    }
}

//  透過遞迴去free B+ tree
void free_internal_node(struct bplus_tree *root, int order)
{
    if (root != NULL)
    {
        if (root->is_leaf)
        {
            free_bplus_tree_node(root);
        }
        else
        {
            for (int i = 0; i < order; i++)
            {
                free_internal_node(root->children[i], order);
            }

            free(root);
        }
    }
}

//  結束program的時候 要釋放記憶體
void quit(struct bplus_tree *root, int order)
{
    free_internal_node(root, order);
}

int main(int argc, char *argv[])
{
    //  order為tree的degree --> key的個數為order - 1, capacity為2 * order
    int order = atoi(argv[1]);
    // struct linked_list *head = NULL;
    struct bplus_tree *root123 = NULL;

    for (int i = 2; i < argc; i++)
    {
        printf("insert value: %d\n", atoi(argv[i]));
        root123 = insert(root123, atoi(argv[i]), order);
        Display(root123, order, 0);
    }

    printf("b+ tree:\n");
    Display(root123, order, 0);
    quit(root123, order);

    return 0;
}
