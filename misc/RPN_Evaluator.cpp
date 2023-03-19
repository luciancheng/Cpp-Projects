#include <cstdlib>
#include <iostream>
using namespace std;

// this program will evaluate mathematical expressions in the form of reverse polish notation through the use of a stack ADT

// Class prototypes
class Stack;

class SNode;

class RPNEvaluator;

// Class implementation
class SNode {
    private:
        std::string elem;
        SNode * next;
        friend class Stack;
};

class Stack {
    public:
        Stack();
        int Size(); 
        bool empty(); 
        void push(const std::string elem);
        std::string pop();
        void print();
    private:
        int size;
        SNode * head;
};

class RPNEvaluator {
    public:
        float Evaluate();
        RPNEvaluator(std::string eqn);
    private:
        std::string equation;
};

// Methods for RPNEvaluator
RPNEvaluator::RPNEvaluator(std::string eqn) : equation(eqn) {} // constructor class for RPNEvaluator to store the equation

float RPNEvaluator::Evaluate() {
    Stack S; // creates a stack S
    for (int i = 0; i < equation.length(); i++) { // iterate through each character
        if (!isspace(equation[i])) { // checking if the character is a space
            if (isdigit(equation[i])) { // if its a digit it will push it onto the stack
                std::string digit = "";
                digit = equation[i];
                S.push(digit);
            } 
            else if ((equation[i] == '+') || (equation[i] == '-') || (equation[i] == '*') || (equation[i] == '/')) { // if its an operator run this code
                float num2;
                float num1;

                // check to see if there are at least 2 numbers present in the stack when an operator is reached and pops the last item and conversts the string to a float
                if (!S.empty()) {
                    num2 = stof(S.pop());
                } else {
                    cout << "Error: malformed expression" << endl;
                    return -1;
                }

                if (!S.empty()) {
                    num1 = stof(S.pop());
                } else {
                    cout << "Error: malformed expression" << endl;
                    return -1;
                }
                //perform the operations
                float nextnum;
                if (equation[i] == '+') {
                    nextnum = num1 + num2;
                }
                else if (equation[i] == '-') {
                    nextnum = num1 - num2;
                }
                else if (equation[i] == '*') {
                    nextnum = num1 * num2;
                }
                else if (equation[i] == '/') {
                    nextnum = num1 / num2;  
                }
                // push the number after the operation back onto the stack by converting the float to a string
                S.push(to_string(nextnum));
            }
            else { // check if anything else other than a number or operator is added
                cout << "Error: malformed expression" << endl;
                return -1;
            }
        }
    } //cout << endl;
    // once the code is done iterating through the equation, it wil pop the item out of the stack
    float finalvalue = stof(S.pop());
    // if its empty, that means its a properly formed equation, if its not empty that means there are extra numbers not used in the operator, so malformed expression
    if (S.empty()) {
        cout << finalvalue << endl;
        return finalvalue;
    } else {
        cout << "Error: malformed expression" << endl;
        return -1;
    }
}
// Methods for Stack
Stack::Stack() { // stack constructor
    size = 0;
    head = NULL;
}

bool Stack::empty() {
    if (head == NULL) {
        return true;
    }
    return false;
}

int Stack::Size() {
    return size;
}

std::string Stack::pop() { // pop and return the item popped out of the stack
    SNode * cur = head;
    std::string value;
    // does nothing if its empty
    if (empty()) {
        return "Empty";
    }
    // popping if there is only one item in the stack
    if (size == 1) {
        size--;
        value = cur->elem;
        delete cur;
        head = NULL;
        return value;
    }
    //iterate until you reach the second last item of the stack and pop the last one
    while (cur->next->next != NULL) {
        cur = cur->next;
    }
    size--;
    value = cur->next->elem;
    delete cur->next;
    cur->next = NULL;
    return value;
}

void Stack::push(const std::string elem) { // method for pushing items onto the stack
    size++;
    if (empty()) {
        head = new SNode;
        head->elem = elem;
        head->next = NULL;
        return;
    }
    SNode * cur = head;
    // add a new node at the end
    while (cur->next != NULL) {
        cur = cur->next;
    }
    cur->next = new SNode;
    cur->next->elem = elem;
    cur->next->next = NULL;
}

void Stack::print() { // print function to check if the stack is working as intended
    SNode * cur = head;
    while (cur != NULL) {
        cout << cur->elem << " -> ";
        cur = cur->next;
    } cout << endl;
}

int main() {
    std::string rpn_exp;
    std::cout << "Please enter an expression in RPN format: ";
    std::getline(std::cin, rpn_exp);
    RPNEvaluator rpn(rpn_exp);
    rpn.Evaluate();
    return EXIT_SUCCESS;
}   
