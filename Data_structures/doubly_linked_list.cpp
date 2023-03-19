#include <cstdlib>
#include <iostream>

typedef std::string Elem;
using namespace std;

class DNode {  // doubly linked list node
 private:
  Elem elem;                 // node element value
  int score;
  DNode* prev;               // previous node in list
  DNode* next;               // next node in list
  friend class DLinkedList;  // allow DLinkedList access
};

class DLinkedList {  // doubly linked list
 public:
  DLinkedList();                 // constructor
  DLinkedList(DLinkedList& dll);
  ~DLinkedList();                // destructor
  bool empty() const;            // is list empty?
  const Elem& front() const;     // get front element
  const Elem& back() const;      // get back element
  void addFront(const Elem& e);  // add to front of list
  void addBack(const Elem& e);   // add to back of list
  void removeFront();            // remove from front
  void removeBack();             // remove from back
  DNode* getHeader();
  void AddScoreInOrder(Elem name, int score);
  void Print();
  void actualPrint(DNode * cur);
  int Size();
  int countsize(DNode * cur);
  void RemoveScore(int index);
  bool UpdateScore(Elem name, int score);
  void operator = (const DLinkedList &dll);
  void OrderByName();
 private:                        // local type definitions
  DNode* header;                 // list sentinels
  DNode* trailer;
 protected:                           // local utilities
  void add(DNode* v, const Elem& e);  // insert new node before v
  void remove(DNode* v);              // remove node v
};

DLinkedList::DLinkedList() {  // constructor
  header = new DNode;         // create sentinels
  trailer = new DNode;
  header->next = trailer;  // have them point to each other
  trailer->prev = header;
  header->prev = NULL;
  trailer->next = NULL;
}

DLinkedList::~DLinkedList() {  // destructor
  while (!empty()) {
    removeFront();  //  remove all but sentinels
  }
  delete header;  //  remove the sentinels
  delete trailer;
}

bool DLinkedList::empty() const {  // is list empty?
  return (header->next == trailer);
}

const Elem& DLinkedList::front() const {  // get front element
  return header->next->elem;
}

const Elem& DLinkedList::back() const {  // get back element
  return trailer->prev->elem;
}

void DLinkedList::add(DNode* x, const Elem& value) {
  DNode* newNode = new DNode;
  newNode->elem = value;
  newNode->next = x->next;
  newNode->prev = x;
  x->next->prev = newNode;
  x->next = newNode;
}

void DLinkedList::addFront(const Elem& e) {  // add to front of list
  add(header, e);
}

void DLinkedList::addBack(const Elem& e) {  // add to back of list
  add(trailer->prev, e);
}

void DLinkedList::remove(DNode* v) {  // remove node v
  DNode* u = v->prev;                 // predecessor
  DNode* w = v->next;                 // successor
  u->next = w;                        // unlink v from list
  w->prev = u;
  delete v;
}

void DLinkedList::removeFront() {  // remove from font
  remove(header->next);
}

void DLinkedList::removeBack() {  // remove from back
  remove(trailer->prev);
}

void listReverse(DLinkedList& L) {  // reverse a list
  DLinkedList T;                    // temporary list
  while (!L.empty()) {              // reverse L into T
    Elem s = L.front();
    L.removeFront();
    T.addFront(s);
  }
  while (!T.empty()) {  // copy T back to L
    Elem s = T.front();
    T.removeFront();
    L.addBack(s);
  }
}

//NEW CODE

void DLinkedList::AddScoreInOrder(Elem name, int score) {
    DNode * newnode = new DNode;
    newnode->elem = name;
    newnode->score = score;
    if (newnode == NULL) { // checks if the allocating memory failed or not
        return;
    }
    if (empty()) { // checks to see if the list is currently empty and will run a set of instrucitons if thats the case
        newnode->prev = header;
        newnode->next = trailer; 
        header->next = newnode;
        trailer->prev = newnode;
    } else { // not empty
        DNode * cur = header->next; //set cur to first node; 
        bool greaterthan = false; // intializies a variable that checks if the new node is greater than the first entry
        if (score > cur->score) { // checks if the new score to be added is the largest item in the current list
            newnode->next = cur;
            newnode->prev = header;
            header->next = newnode;
            cur->prev = newnode;
            greaterthan = true; 
        }
        if (!greaterthan) { // if the item is not the biggest it will run this code
            while ((score < cur->score) && (cur != trailer)) { // iterate through the list to find out where to slot the new score entry
                cur = cur->next;
            }
            DNode * oldprev = cur->prev;
            newnode->next = cur;
            newnode->prev = oldprev;
            cur->prev = newnode;
            oldprev->next = newnode;
        }
    }

}

void DLinkedList::Print() {
    actualPrint(header); // helper funciton that actually does the recursive print
}

void DLinkedList::actualPrint(DNode * cur) { // actual print funciton
    if (empty()) { // check if it's empty
        return;
    } 
    if (cur == header) { // if its header it will advance to the first actual node
        cur = cur->next;
    }
    if (cur->next == trailer) { // if the cur->next is trailer then it will print the ending print
        cout<<"{"<<cur->elem<<","<<cur->score<<"}"<<endl;
        return;
    }
    cout<<"{"<<cur->elem<<","<<cur->score<<"}"<<"->"; // run this print until it reaches the last node to be printed
    actualPrint(cur->next); // recursive call
}

int DLinkedList::Size() {
    return countsize(header); // helper function
}

int DLinkedList::countsize(DNode * cur) { // counting the actual size
    if (cur->next == trailer) { // checking if its the last node to be counted
        return 0;
    }
    return countsize(cur->next) + 1; // recursivelcall adding 1 each time the funciton is called
}

void DLinkedList::RemoveScore(int index) {
    int currentindex = 0; // intialize a variable to keep track of the current index that the function is on when iterating through the lsit
    DNode * cur = header->next; // sets cur to be the firs tnode
    while (currentindex != index) { // kep running this while loop until the current index meatches the index while iterating through the array
        cur = cur->next;
        currentindex++;
    }
    cur->prev->next = cur->next; // reassign the pointers
    cur->next->prev = cur->prev;
    delete cur; // deletes the current node
}

bool DLinkedList::UpdateScore(Elem name, int score) {
    DNode * cur = header->next; // intializes the cur to be the first actual node
    while (cur != trailer) { // keeps iterating until it reaches the trailer or it reaches the name of the person we want to change
        if (cur->elem == name) { // checks if the name matches
            cur->next->prev = cur->prev;
            cur->prev->next = cur->next;
            AddScoreInOrder(cur->elem, score); // readds the score and updates the list into the correct descending order
            delete cur; // deletes the current node
            //cur->score = score; // updates their score

            return true; // returns true if it does to break out of the function
        }
        cur = cur->next; // keeps advancing
    }
    return false; // returns false if it isnt found
}

DLinkedList::DLinkedList(DLinkedList& dll){ // copy constructor
  header = new DNode;         // create sentinels
  trailer = new DNode;
  header->next = trailer;  // have them point to each other
  trailer->prev = header;
  header->prev = NULL;
  trailer->next = NULL;

  DNode * cur = dll.header; // sets cur to be the header of the passed in dll
  cur = cur->next; // starts at the first node
  while (cur != dll.trailer) { // keeps going until cur reaches the trailer
      AddScoreInOrder(cur->elem, cur->score); // adds score in order
      cur = cur->next; // advances cur
  }
}

DNode * DLinkedList::getHeader() {
    return header;
}

void  DLinkedList::OrderByName() {
    if ((header->next == trailer) || (header->next->next == trailer)) return; // checks to see if the list is empty or has only one item in it, if so it will return and not do anything
    DNode * cur1 = header->next;
    DNode * tail = trailer->prev; // tail to tell when the innter loop to stop

    // sorting algorithm based on bubble sort
    while (cur1->next != trailer) { // itearting until it reaches the second last element of the list
        DNode * cur2 = header->next;
        while (cur2 != tail) {
            //cout<<cur2->elem<< " compare with " << cur2->next->elem << endl;
            if (cur2->elem > cur2->next->elem) { // checks alphabetically
                //cout<<cur2->elem<< " swap with " << cur2->next->elem << endl;

                // just swapping the name and score
                string temp = cur2->next->elem; // swaps the items
                int tempscore = cur2->next->score;
                cur2->next->elem = cur2->elem;
                cur2->next->score = cur2->score;
                cur2->elem = temp;
                cur2->score = tempscore;
            }
            cur2 = cur2->next; // advances cur2
        }
        tail = tail->prev;
        cur1 = cur1->next; // advances both of these
    }
}

// overloading assignment = operator
void DLinkedList::operator = (const DLinkedList& dll) {
    cout<<"Running Assignment overload"<<endl;

    header = new DNode;         // create sentinels
    trailer = new DNode;
    header->next = trailer;  // have them point to each other
    trailer->prev = header;
    header->prev = NULL;
    trailer->next = NULL;

    DNode * cur = dll.header; // does the same thing as the copy constructor
    cur = cur->next;
    while (cur != dll.trailer) {
        AddScoreInOrder(cur->elem, cur->score);
        cur = cur->next;
    }
}


int main() {
    DLinkedList scores;
    scores.AddScoreInOrder("Jeff", 7);
    scores.AddScoreInOrder("Jen", 9);
    scores.AddScoreInOrder("Ilya", 3);
    scores.AddScoreInOrder("Sara", 10);
    scores.AddScoreInOrder("Sam", 11);

    // Test size function
    cout << "Number of scores is " << scores.Size() << endl;
    scores.Print();
    // Test remove function
    scores.RemoveScore(3);
    cout << "Number of scores is now " << scores.Size() << endl;
    scores.Print();
    // Test update function
    if (scores.UpdateScore("Jeff",6))
    scores.Print();
    // Test copy construcor
    DLinkedList scores_copy_1(scores);
    scores.UpdateScore("Jen",5);
    scores.Print();
    scores_copy_1.Print();
    // Test assignment operator overload
    DLinkedList scores_copy_2; 
    scores_copy_2 = scores_copy_1;
    scores_copy_1.UpdateScore("Jen",5);
    scores_copy_1.Print();
    scores_copy_2.Print();
    // Test OrderByName function
    scores_copy_2.OrderByName();
    scores_copy_2.Print();//*/
}
