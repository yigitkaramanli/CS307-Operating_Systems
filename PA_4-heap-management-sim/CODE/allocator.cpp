
#include <pthread.h>
#include <unistd.h>
#include <iostream>
#include <list>

using namespace std;

#ifndef _HEAP_MANAGER
#define _HEAP_MANAGER

struct HeapNode
{
    /* data */
    int id;  //ID of the thread allocated that memory
    int size;   // size of the allocated chunk
    int index;      // starting index 
    HeapNode *next;
    HeapNode() {}
    HeapNode(int id, int size, int index, HeapNode *node) : id(id), size(size), index(index), next(node) {}
};

class HeapManager
{
private:
    /* data */
    HeapNode *head;  // head of the linked list
    pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER; //Coarse grained lock used to prevent data races on the object

public:
    HeapManager()
    {
        head = new HeapNode(); // create the first node
    }
    ~HeapManager()
    {
        recursiveDelete(head);   //recursively delete the linked list
        pthread_mutex_destroy(&lock);   // destroy the lock
    }

    void recursiveDelete(HeapNode *node)
    {
        if (node != NULL)
        {
            recursiveDelete(node->next);
            delete node;
            node = NULL;
        }
    }

    //Print function
    void print()
    {
        HeapNode *ptr = head;  //create the pointer to iterate over the list 
        while (ptr != NULL)
        {
            cout << "[" << ptr->id << "][" << ptr->size << "][" << ptr->index << "]";
            ptr = ptr->next;
            if (ptr != NULL)
            {
                cout << "---";
            }
        }
        cout << endl;
    }

    //initialize the heap with the given size 
    int initHeap(int size)
    {
        head->id = -1;
        head->size = size;
        head->index = 0;
        print();
        return 1;
    }
    //allocator function
    int myMalloc(int ID, int size)
    {
        pthread_mutex_lock(&lock); //acquire the lock on the linked list
        HeapNode *ptr = head;
        int index;
        while (ptr != NULL) // iterate forward on the list until finding an available 'unallocated' node
        {
            if (ptr->id == -1 && ptr->size >= size)
            {
                break;
            }
            ptr = ptr->next;
        }
        if (ptr == NULL) // if no such nodes found, print appropriate error message and return -1
        {
            cout << "Can not allocate, requested size " << size << " for thread " << ID << " is bigger than remaining size\n";
            print();
            pthread_mutex_unlock(&lock);
            return -1;
        }
        else // if there is such a node
        {
            if (ptr->size == size) //if the size of the node is the same as the requested size 
            {
                ptr->id = ID;  // Just change the ID from -1 to tid
                index = ptr->index; // set the return value
            }
            else //If the size of the node is larger than the requested size: Divide the node
            {
                int nextSize = ptr->size - size; //The size of the new node 
                int nextIndex = ptr->index + size; // Index of the new node
                ptr->id = ID;   //set the ID of the node as tid
                ptr->size = size; //set the size 
                HeapNode *nextnode = new HeapNode(-1, nextSize, nextIndex, ptr->next); //create a new node 
                ptr->next = nextnode; //insert the newly created node into the list 
                index = ptr->index;     // set the return value
            }
        }
        cout << "Allocated for thread " << ID << endl;   
        print();
        pthread_mutex_unlock(&lock);
        return index;
    }

    //Function that 'frees' the chunks
    int myFree(int ID, int index)
    {
        pthread_mutex_lock(&lock); //acquire the lock on the object
        HeapNode *previousNode = NULL; //node to check tehe previous node
        HeapNode *ptr = head;
        while (ptr != NULL )  //iterate over the list until you found the node or reach the end 
        {
            if (ptr->id == ID && ptr->index == index){
                break;
            }
            previousNode = ptr;
            ptr = ptr->next;
        }
        if (ptr == NULL) //if no such node found, print the error and return -1
        {
            cout << "Thread ID: " << ID << ", Index: " << index << " could not be found. Free operation failed.\n";
            print();
            pthread_mutex_unlock(&lock);
            return -1;
        }
        else //if such a node is found
        {
            ptr->id = -1; //set it as 'Free'
            if (ptr->next != NULL && ptr->next->id == -1) //check if the next node is also free
            {                                                   // if so: Merge them 
                ptr->size += ptr->next->size;
                HeapNode * tmp = ptr->next->next;
                delete ptr->next;
                ptr->next = tmp;
            }
            if (previousNode != NULL && previousNode->id == -1)//check if the previous node is also free
            {                                                   // if so: Merge them
                previousNode->size += ptr->size;
                previousNode->next = ptr->next;
                delete ptr;
            }
            cout << "Freed for thread " << ID << "\n";
            print();
            pthread_mutex_unlock(&lock);
            return 1;
        }
    }
};
#endif