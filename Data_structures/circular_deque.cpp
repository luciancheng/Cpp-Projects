#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

// class prototype
class Deque;

class Deque {
    public:
        Deque(int n);
        void insertFront(std::string e);
        void insertBack(std::string e);
        void eraseFront();
        void eraseBack();
        std::string front();
        std::string back();
        int size();
        bool empty();
        void print();
        void printv();
    private:
        int Size;
        int startindex;
        int endindex;
        int capacity;
        vector<std::string> V;
};

Deque::Deque(int n) : Size(0), startindex(0), endindex(0), V(n), capacity(n) {} // constructor for the Deque class

void Deque::insertFront(std::string e) { // function to insert an elmeent at the front of the deque and shifts the startindex
    if (Size == capacity) {
        return;
    }
    startindex = (startindex - 1 + capacity) % capacity;
    V[startindex] = e;
    Size++;
}

void Deque::insertBack(std::string e) { // function to insert an element at the end of the deque and shifts the endindex
    if (Size == capacity) {
        return;
    }
    V[endindex] = e;
    endindex = (endindex + 1) % capacity;
    Size++;
}

void Deque::eraseFront() { //  removes the front element and moves the start index
    if (Size == 0) {
        return;
    }
    startindex = (startindex + 1) % capacity;
    Size--;
}

void Deque::eraseBack() { // removes the end element and moves the end index
    if (Size == 0) {
        return;
    }
    endindex = (endindex - 1 + capacity) % capacity;
    Size--;
}

std::string Deque::front() { // retrieves the front item
    if (empty()) {
        return "error: empty deque";
    }
    return V[startindex];
}

std::string Deque::back() { // retrieves the rear item by taking the endindex and circulating it around the vector
    if (empty()) {
        return "error: empty deque";
    }
    int index = (endindex - 1 + capacity) % capacity; 
    return V[index];
}

bool Deque::empty() { // checks if its empty based off its size
    if (Size == 0) {
        return true;
    }
    return false;
}

int Deque::size() { // accesor method for size
    return Size;
}

// additional functions that were not required but added to check if the deque and vector were working as intended
void Deque::print() {
    int currentindex = startindex;
    for (int i = 0; i < Size; i++) {
        cout << V[currentindex] << " -> ";
        currentindex = (currentindex + 1) % capacity;
    }cout << endl;
}

void Deque::printv() {
    for (int i =0; i < capacity; i++) {
        cout << V[i] << " ";
    } cout << endl;
}
