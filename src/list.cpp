#include <list.h>
#include <memorymanagement.h>
#include <monitor.h>

template<class T>
LinkedList<T>::LinkedList()
: head(nullptr), current(nullptr)
{

}

template<class T>
void LinkedList<T>::add(T data) {
    node* link = (node*) new node;
    link->data = data;
    link->next = nullptr;
    current->next = link;
    if (head == nullptr) {
        head = link;
    }
    current = link;
}

template<class T>
int LinkedList<T>::size() {
    node* current = head;
    int i = 0;
    while(current != nullptr) {
        current = current->next;
        i++;
    }
    return i;
}

template<class T>
T LinkedList<T>::remove(int index) {
    node* prev = nullptr;
    node* current = head;
    int i = 0;
    while(current != nullptr) {
        if (i == index) {
            if (prev == nullptr) {
                //Remove first element
                head = current->next;
            }
            else {
                prev->next = current->next;
            }
            T data = current->data;
            delete[] current;
            return data;
        }
        prev = current;
        current = current->next;
        i++;
    }
    return 1234567;
}

template<class T>
T LinkedList<T>::get(int index)
{
    node* current = head;
    int i = 0;
    while(current != nullptr) {
        if (i == index)
            return current->data;
        current = current->next;
        i++;
    }
    return 1234567;
}
