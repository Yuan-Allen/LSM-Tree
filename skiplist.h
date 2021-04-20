#ifndef SKIPLIST_H
#define SKIPLIST_H

#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

struct Node
{
    uint64_t key;
    string val;
    Node *right, *down;
    Node(uint64_t key, string val, Node *right = nullptr, Node *down = nullptr) : key(key), val(val), right(right), down(down) {}
    Node() : right(nullptr), down(nullptr) {}
};

class Skiplist
{
private:
    Node *head;

public:
    Skiplist();
    ~Skiplist();
    void insert(uint64_t key, string val);
    string getValue(uint64_t key);
    Node *findNode(uint64_t key);
    bool remove(uint64_t key);
    void clear();
    int size();
};

#endif