#include <stdio.h>
#include <stdlib.h>

typedef struct BPlusNode
{
    int *keys;
    struct BPlusNode **pointers;
    struct BPlusNode *parent;
    int num_keys;
    int is_leaf;
    struct BPlusNode *prev;
    struct BPlusNode *next;
} BPlusNode;

BPlusNode *createBPlusTree()
{
    BPlusNode *root = (BPlusNode *)malloc(sizeof(BPlusNode));
    root->keys = NULL;
    root->pointers = NULL;
    root->parent = NULL;
    root->num_keys = 0;
    root->is_leaf = 1;
    root->prev = NULL;
    root->next = NULL;
    return root;
}

void insertKey(BPlusNode **root, int key)
{
    if (*root == NULL)
    {
        BPlusNode *leaf = (BPlusNode *)malloc(sizeof(BPlusNode));
        leaf->keys = (int *)malloc(sizeof(int));
        leaf->pointers = NULL;
        leaf->parent = NULL;
        leaf->num_keys = 1;
        leaf->is_leaf = 1;
        leaf->prev = NULL;
        leaf->next = NULL;
        leaf->keys[0] = key;
        *root = leaf;
    }
    else
    {
        BPlusNode *node = *root;
        while (!node->is_leaf)
        {
            int i;
            for (i = 0; i < node->num_keys && key >= node->keys[i]; i++)
                ;
            node = node->pointers[i];
        }

        int i;
        for (i = 0; i < node->num_keys && key > node->keys[i]; i++)
            ;

        node->keys = (int *)realloc(node->keys, (node->num_keys + 1) * sizeof(int));
        for (int j = node->num_keys; j > i; j--)
        {
            node->keys[j] = node->keys[j - 1];
        }

        node->keys[i] = key;
        node->num_keys++;

        if (i == 0 && node->prev != NULL)
        {
            node->prev->next = node;
            node->prev = node->prev;
            node->next = node->prev->next;
            if (node->next != NULL)
            {
                node->next->prev = node;
            }
            node->prev = node;
            node->next = NULL;
        }
    }
}

void displayBPlusTree(BPlusNode *root)
{
    if (root == NULL)
    {
        printf("B+樹為空。\n");
        return;
    }

    BPlusNode *node = root;
    while (!node->is_leaf)
    {
        node = node->pointers[0];
    }

    while (node != NULL)
    {
        for (int i = 0; i < node->num_keys; i++)
        {
            printf("%d ", node->keys[i]);
        }
        node = node->next;
    }

    printf("\n");
}

void quit(struct BPlusNode *root)
{
}

int main(int argc, char *argv[])
{
    BPlusNode *root = createBPlusTree();

    for (int i = 2; i < argc; i++)
    {
        insertKey(&root, atoi(argv[i]));
    }
    // insertKey(&root, 10);
    // insertKey(&root, 20);
    // insertKey(&root, 30);

    displayBPlusTree(root);

    return 0;
}
