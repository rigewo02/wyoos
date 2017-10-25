#ifndef LIST_H
#define LIST_H

template<class T>
class LinkedList {

public:
    LinkedList();
    void add(T data);
    int size();
    T remove(int index);
    T get(int index);
private:
    struct node {
        T data;
        node *next;
    };
    node *head;
    node *current;
};

#include <../src/list.cpp>

#endif
