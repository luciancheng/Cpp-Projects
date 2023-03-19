#include <cstdlib>
#include <iostream>
using namespace std;

class SLinkedList;

SLinkedList createsll(int ** twodarray, int firstindex, int x, int & output);
// class for the node and singly linked list
class SNode {
    private:
        int value;
        SNode * next;
        friend class SLinkedList;
};

class SLinkedList {
    public:
        SLinkedList();              // empty list constructor
        //~SLinkedList();             // destructor
        void addelem(int elem);
        void Print();
    private:
        SNode* head;
};

SLinkedList::SLinkedList() : head(NULL) {}

void SLinkedList::addelem(int elem) { // method for adding a node into the singly linked list
    if (head == NULL) { // runs a certain part of the code if the item about to be added is the only item
        head = new SNode;
        head->value = elem;
        head->next = NULL;
        return;
    }
    SNode * cur = head; // sets to cur to head
    while (cur->next != NULL) { // keeps going until the end of the end
        cur = cur->next;
    }
    cur->next = new SNode; // allocates memory for a new note
    cur->next->value = elem; 
    cur->next->next = NULL; // sets the next item to NULL
}

SLinkedList createsll(int ** twodarray, int firstindex, int x, int & output) { // converts the inputs into a singly linked list
    SLinkedList sll; // makes a class of singly linked list
    int currentindex = firstindex; // start a the first index
    bool found = false; // bool value to know if the smallest value that is greater than x is found
    while (currentindex != -1) {
        sll.addelem(twodarray[0][currentindex-1]); // adds a new element into the singly linked list
        if ((twodarray[0][currentindex-1] >= x) && !found) {
            output = twodarray[0][currentindex-1]; found = true; // implictly returns the number by passing output by refernce, sets found to be true
        }
        currentindex = twodarray[1][currentindex-1]; // moves to the enxt index based on the input
    }
    if (!found) output = -1; // if no number was found than return -1

    return sll; // retuurns the singly linked list that was created
}

void SLinkedList::Print() { // method to print the singly like list iteravely 
    SNode * cur = head;
    while (cur->next != NULL) {
        cout << cur->value << "->";
        cur = cur->next;
    }
    cout << cur->value << endl;
}

int main() {
    int n, first, x;
    cin >> n >> first >> x;

    int ** twodarray = new int*[2];
    twodarray[0] = new int[n]; // element
    twodarray[1] = new int[n]; // index

    for (int i = 0; i < n; i++) {
        cin >> twodarray[0][i] >> twodarray[1][i]; // inputs for the number and next node it points to
    }
    int output;
    SLinkedList sll = createsll(twodarray, first, x, output); // creates an sll with the function
    cout << endl << "The created SLL is: " << endl;
    sll.Print(); cout << endl; // prints the sll
    cout << "The minimum value that is greater than or equal to x = " << x << " is " << output << endl;//*/
    return 0;
}
