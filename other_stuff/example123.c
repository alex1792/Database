#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct bplus_tree
{
    int *key;
    int *val;
    bool is_leaf;
    struct bplus_tree **poninters;
};

void print_preorder(struct bplus_tree *root, int order, int tabCnt)
{
    if (root != NULL)
    {
        for (int i = 0; i < tabCnt; i++)
            printf("\t");
        // printf("\n");
        if (!root->is_leaf)
        {
            printf("(");
            for (int i = 0; i < order - 1; i++)
            {
                if (root->key[i] != 0)
                    printf("%d", root->key[i]);
                else
                    printf("_");
                if (i < order - 2)
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

        for (int i = 0; i < order; i++)
            print_preorder(root->poninters[i], order, tabCnt + 1);
    }
}

struct bplus_tree *initialize(int val, int order, bool isLeaf)
{
    struct bplus_tree *new_node = malloc(sizeof(struct bplus_tree));
    new_node->key = malloc(sizeof(int) * (order - 1));
    new_node->val = malloc(sizeof(int) * order * 2);
    if (!isLeaf)
        new_node->key[0] = val;
    else
        new_node->val[0] = val;
    new_node->is_leaf = isLeaf;
    new_node->poninters = malloc(sizeof(struct bplus_tree *) * order);
    for (int i = 0; i < order; i++)
        new_node->poninters[i] = NULL;
    return new_node;
}

int max(struct bplus_tree *root, int order)
{
    int max = root->val[0];
    for (int i = 1; i < 2 * order; i++)
    {
        if (root->val[i] > max)
            max = root->val[i];
    }
    return max;
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

int count_key_in_node(struct bplus_tree *node, int order)
{
    int cnt = 0;
    for (int i = 0; i < order - 1; i++)
    {
        if (node->key[i] != 0)
            cnt++;
    }
    return cnt;
}

bool leaf_is_full(struct bplus_tree *node, int capacity)
{
    int cnt = count_value_in_leaf_node(node, capacity);
    if (cnt == capacity)
        return true;
    return false;
}

bool node_is_full(struct bplus_tree *node, int order)
{
    int cnt = count_key_in_node(node, order);
    if (cnt == order - 1)
        return true;
    return false;
}

int cmpfunc(const void *a, const void *b)
{
    return (*(int *)a - *(int *)b);
}

int compute_threshold(int order)
{
    if (order % 2 != 0)
        return order / 2 + 1;
    return order / 2;
}

struct bplus_tree *find_insert_node(struct bplus_tree *root, int key, int order)
{
    struct bplus_tree *ptr = root;
    while (ptr)
    {
        for (int i = 0; i < order - 1; i++)
        {
            if (ptr->key[i] > key)
            {
                if (ptr->poninters[i] == NULL)
                    return ptr;
                if (ptr->poninters[i]->key[0] == key)
                    return ptr;
                ptr = ptr->poninters[i];
                break;
            }
        }
        if (key >= ptr->key[order - 2])
        {
            if (ptr->poninters[order - 1] == NULL)
                return ptr;
            else
                ptr = ptr->poninters[order - 1];
        }
    }
    return NULL;
}

struct bplus_tree *find_parent_node(struct bplus_tree *root, struct bplus_tree *current, int order)
{
    if (root == current)
        return NULL;

    struct bplus_tree *ptr = root;
    int key = current->key[0];
    while (ptr)
    {
        for (int i = 0; i < order - 1; i++)
        {
            if (ptr->key[i] > key)
            {
                if (ptr->poninters[i] == current)
                    return ptr;
                ptr = ptr->poninters[i];
            }
        }
        if (key >= ptr->key[order - 2])
        {
            if (ptr->poninters[order - 1] == current)
                return ptr;
            ptr = ptr->poninters[order - 1];
        }
    }
    return ptr;
}

struct bplus_tree *split(struct bplus_tree *root, struct bplus_tree *node, struct bplus_tree *new_node, int value, int order)
{
    if (node == NULL)
    {
        struct bplus_tree *new_root = malloc(sizeof(struct bplus_tree));
        new_root->key = malloc(sizeof(int) * (order - 1));
        new_root->key[0] = value;
        new_root->poninters = malloc(sizeof(struct bplus_tree) * order);
        new_root->poninters[0] = root;
        new_root->poninters[1] = new_node;
        return new_root;
    }

    if (node_is_full(node, order))
    {
        //  拆兩部分
        int *values = malloc(sizeof(int) * order);
        for (int i = 0; i < order - 1; i++)
            values[i] = node->key[i];
        values[order - 1] = value;
        qsort(values, order, sizeof(int), cmpfunc);

        //  前半部分保留在原父節點
        int threshold = compute_threshold(order - 1);
        for (int i = 0; i < threshold; i++)
            node->key[i] = values[i];
        for (int i = threshold; i < order - 1; i++)
            node->key[i] = 0;
        //  後半部分移至新建的父節點
        struct bplus_tree *new_parent_node = malloc(sizeof(struct bplus_tree));
        new_parent_node->key = malloc(sizeof(int) * (order - 1));
        new_parent_node->is_leaf = false;
        new_parent_node->poninters = malloc(sizeof(struct bplus_tree *) * order);

        for (int i = threshold; i < order; i++)
        {
            new_parent_node->key[i - threshold] = values[i];
            printf("values[%d]: %d\n", i, values[i]);
        }

        //  調整pointers
        if (new_node != NULL)
        {
            int new_key_cnt = count_key_in_node(node, order);
            printf("new_key_cnt: %d\n", new_key_cnt);
            int i = 0;
            for (i = 0; i < order - new_key_cnt - 1; i++)
            {
                printf("node->pointers[%d]: %d\n", new_key_cnt + i + 1, node->poninters[new_key_cnt + i + 1]->key[0]);
                new_parent_node->poninters[i] = node->poninters[new_key_cnt + i + 1];
                printf("connect\n");
            }
            new_parent_node->poninters[i] = new_node;
            printf("connect\n");
        }

        //  insert回parent node
        struct bplus_tree *parent = find_parent_node(root, node, order);
        if (parent)
            printf("parent->key: %d\n", parent->key[0]);
        return split(root, parent, new_parent_node, new_parent_node->key[0], order);
        // return root;
    }
}

/*
insert into B+ tree
1. 搜尋適當的葉節點，將鍵值插入該葉節點中的正確位置
    2. 如果葉節點已滿，進行分裂操作
        a. 將葉節點分成兩半，前半部分保留在原葉節點，後半部分移至新建的葉節點
        b. 將新葉節點插入到原葉節點的右邊
        c. 將新葉節點的最小鍵值提升到父節點中的正確位置
        d. 如果父節點已滿，遞歸進行分裂操作
    3. 如果插入操作引起了父節點的分裂，遞歸向上調整
        a. 將父節點分成兩半，前半部分保留在原父節點，後半部分移至新建的父節點
        b. 將新父節點插入到原父節點的右邊
        c. 將新父節點的最小鍵值提升到更高層父節點中的正確位置
        d. 如果更高層的父節點已滿，遞歸進行分裂操作
    4. 如果插入操作引起了根節點的分裂，則建立新的根節點
        a. 創建一個新的根節點，將原根節點和新分裂出來的父節點作為其子節點
        b. 將新根節點的最小鍵值設置為原根節點的最小鍵值
*/
// struct bplus_tree *insert(struct bplus_tree *root, int value, int order)
// {
//     if (root->is_leaf)
//     {
//         if (leaf_is_full(root, 2 * order))
//         {
//             split();
//         }
//     }
// }

int main()
{
    int order = 3;
    struct bplus_tree *root = initialize(22, order, false);
    // root->val[1] = 20;

    struct bplus_tree *left = initialize(12, order, false);
    struct bplus_tree *mid = initialize(31, order, false);
    // struct bplus_tree *right = initialize(25, order);

    root->poninters[0] = left;
    root->poninters[1] = mid;
    // root->poninters[2] = right;

    struct bplus_tree *list1 = initialize(1, order, true);
    list1->val[1] = 8;
    list1->val[2] = 11;
    left->poninters[0] = list1;

    struct bplus_tree *list2 = initialize(12, order, true);
    list2->val[1] = 16;
    list2->val[2] = 17;
    list2->val[3] = 18;
    list2->val[4] = 19;
    left->poninters[1] = list2;

    struct bplus_tree *list3 = initialize(22, order, true);
    list3->val[1] = 23;
    list3->val[2] = 28;
    mid->poninters[0] = list3;

    struct bplus_tree *list4 = initialize(31, order, true);
    list4->val[1] = 41;
    list4->val[2] = 52;
    list4->val[3] = 58;
    list4->val[4] = 59;
    list4->val[5] = 61;
    mid->poninters[1] = list4;

    print_preorder(root, order, 0);

    struct bplus_tree *node = initialize(5, order, true);
    node->val[1] = 10;
    node->val[2] = 15;

    struct bplus_tree *node2 = initialize(20, order, true);
    node2->val[1] = 25;
    node2->val[2] = 30;
    node2->val[3] = 35;

    struct bplus_tree *new_root = initialize(max(node2, order), order, false);
    new_root->poninters[0] = node;
    new_root->poninters[1] = node2;
    print_preorder(new_root, order, 0);

    struct bplus_tree *root123 = initialize(22, order, false);
    root123->key[1] = 58;
    struct bplus_tree *node_left = initialize(12, order, false);
    root123->poninters[0] = node_left;
    struct bplus_tree *node_mid = initialize(31, order, false);
    root123->poninters[1] = node_mid;
    struct bplus_tree *node_right = initialize(58, order, false);
    node_right->key[1] = 70;
    root123->poninters[2] = node_right;
    print_preorder(root123, order, 0);

    // struct bplus_tree *new_node = initialize(70, order, false);
    // new_node->key[1] = 84;
    // node_right->key[1] = 0;

    root = split(root123, node_right, NULL, 84, order);
    print_preorder(root, order, 0);
    // printf("%d\n", node_is_full(node_right, order));

    struct bplus_tree *target = find_insert_node(root123, 60, order);
    printf("target->key: %d\n", target->key[0]);

    struct bplus_tree *ret = find_parent_node(root123, node_left, order);
    if (ret)
        printf("ret->key: %d\n", ret->key[0]);

    return 0;
}

//  有split 但沒有接上