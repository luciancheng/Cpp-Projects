#include <cstdlib>
#include <iostream>
using namespace std;

template <typename E>  // forward declaration of SLinkedList
class SLinkedList;

template <typename E>
class SNode {  // singly linked list node
 private:
  E elem;                       // linked list element value
  SNode<E>* next;               // next item in the list
  friend class SLinkedList<E>;  // provide SLinkedList access
};

template <typename E>
class SLinkedList {  // a singly linked list
 public:
  SLinkedList();              // empty list constructor
  ~SLinkedList();             // destructor
  bool empty() const;         // is list empty?
  const E& front() const;     // return front element
  void addFront(const E& e);  // add to front of list
  void removeFront();         //  remove front of list
  SNode<E> * gethead();
  int CountNodes(SNode<E> * cur);
  void print();
  void appendlist(SLinkedList<E>& list);
  void reverselist(SNode<E> * cur);
 private:
  SNode<E>* head;  // head of the list
};

template <typename E>
SLinkedList<E>::SLinkedList()  // constructor
    : head(NULL) {}

template <typename E>
bool SLinkedList<E>::empty() const {  // is list empty?
  return head == NULL;
}

template <typename E>
const E& SLinkedList<E>::front() const {  // return front element
  return head->elem;
}

template <typename E>
SLinkedList<E>::~SLinkedList() {  // destructor
  while (!empty()) removeFront();
}

template <typename E>
void SLinkedList<E>::addFront(const E& e) {  // add to front of list
  SNode<E>* v = new SNode<E>;                // create new node
  v->elem = e;                               // store data
  v->next = head;                            // head now follows v
  head = v;                                  // v is now the head
}

template <typename E>
void SLinkedList<E>::removeFront() {  // remove front item
  if (head == NULL) return;
  SNode<E>* old = head;  // save current head
  head = old->next;      // skip over old head
  delete old;            // delete the old head
}

// ADDED CODE
template <typename E>
SNode<E> * SLinkedList<E>::gethead() { // helper funciton for getting the head of the sll
    return head;
}

template <typename E>
int SLinkedList<E>::CountNodes(SNode<E> * cur) { 
    if (cur == NULL) { // base case if cur == NULL to stop counting
        return 0; // returns 0
    }
    return 1 + CountNodes(cur->next); // recursive call adding 1 each time to count
}

template <typename E>
void SLinkedList<E>::print() { 
    SNode<E> * cur = head; // sets cur to be the head of the sll
    while ((cur->next) != NULL) { // while loop for iterating through each item in the sll until it reaches the end
        cout << cur->elem << "->";
        cur = cur->next; // advances cur
    }
    cout<<cur->elem<<endl;
}
template <typename E> 
void SLinkedList<E>::appendlist(SLinkedList<E>& list) {
    SNode<E> * cur = head;
    while (cur->next != NULL) {
        cur = cur->next; // find the end of the linked list
    }
    SNode<E> * cur2 = list.gethead(); // make a new cur pointer to the head of the list to be appended

    while (cur2 != NULL) { // append the list one node at a time
        cur->next = new SNode<E>;
        cur->next->elem = cur2->elem;
        cur = cur->next;
        cur2 = cur2->next;
    }
    cur->next = NULL; // sets the end pointer to NULL
}

template <typename E> 
void SLinkedList<E>::reverselist(SNode<E> * cur) {
    if (cur->next == NULL) { // base case
        head = cur; // sets head to be the last item in the list
        return;
    }
    reverselist(cur->next); // recursive call
    cur->next->next = cur; // makes the next item in the list the correct node
    cur->next = NULL; // makes the next pointer null
}


int main() {
  SLinkedList<string> sll;
  sll.addFront("four");
  sll.addFront("three");
  sll.addFront("two");
  sll.addFront("one");
  cout << "List 1 has " << sll.CountNodes(sll.gethead()) << " nodes:" << endl;
  sll.print();

  SLinkedList<string> sll2;
  sll2.addFront("seven");
  sll2.addFront("six");
  sll2.addFront("five");
  cout << "List 2 has " << sll2.CountNodes(sll2.gethead()) << " nodes:" << endl;
  sll2.print();
  sll.appendlist(sll2);
  cout << "After appending List 2 to List 1, List 1 has " << sll.CountNodes(sll.gethead()) << " nodes:" << endl;
  sll.print();
  cout << "After reversing list 1, it looks like this:" << endl;
  sll.reverselist(sll.gethead());
  sll.print();
}
