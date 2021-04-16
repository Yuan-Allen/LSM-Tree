#include "skiplist.h"

Skiplist::Skiplist()
{
    head = new Node(0, "header");
}

Skiplist::~Skiplist()
{
    clear();
}

void Skiplist::insert(uint64_t key, string val)
{
    vector<Node *> pathList; //从上至下记录搜索路径
    bool existFlag = false;  //该key对应的节点已经存在？
    Node *p = head;
    while (p)
    {
        while (p->right && p->right->key < key)
        {
            p = p->right;
        }
        if (p->right && p->right->key == key)
        {                        //如果查找到已经存在
            p->right->val = val; //覆盖其val
            existFlag = true;    //修改existFlag为true
        }
        pathList.push_back(p);
        p = p->down;
    }
    if (existFlag)
        return; //该key对应的节点已经存在，且值已经覆盖完毕，可以直接return

    bool insertUp = true;
    Node *downNode = nullptr;
    while (insertUp && pathList.size() > 0)
    { //从下至上搜索路径回溯，50%概率
        Node *insertNode = pathList.back();
        pathList.pop_back();
        insertNode->right = new Node(key, val, insertNode->right, downNode); //add新结点
        downNode = insertNode->right;                                        //把新结点赋值为downNode
        insertUp = (rand() & 1);
    }
    if (insertUp)
    { //插入新的头结点，加层
        Node *oldHead = head;
        head = new Node();
        head->right = new Node(key, val, nullptr, downNode);
        head->down = oldHead;
    }
}

string Skiplist::getValue(uint64_t key)
{
    Node *p = findNode(key);
    if (p)
    {
        return p->val;
    }
    else
    {
        return "";
    }
}

Node *Skiplist::findNode(uint64_t key)
{
    Node *p = head;
    while (p)
    {
        while (p->right && p->right->key < key)
        {
            p = p->right;
        }
        if (p->right && p->right->key == key)
            return p->right;
        p = p->down;
    }
    return nullptr;
}

bool Skiplist::remove(uint64_t key)
{
    Node *p = head, *tmp = nullptr;
    bool flag = false;
    while (p)
    {
        while (p->right && p->right->key < key)
        {
            p = p->right;
        }
        if (p->right && p->right->key == key)
        {
            tmp = p->right;
            p->right = tmp->right;
            delete tmp;
            flag = true;
            if (p == head && p->right == nullptr)
            {
                tmp = head;
                head = head->down;
                delete tmp;
                p = head;
                continue;
            }
        }
        p = p->down;
    }
    return flag;
}

void Skiplist::clear()
{
    Node *curr = head, *tmp = nullptr;
    while (curr)
    {
        while (curr->right)
        {
            tmp = curr->right;
            curr->right = tmp->right;
            delete tmp;
        }
        tmp = curr;
        curr = curr->down;
        delete tmp;
    }
}