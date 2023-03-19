#include <iostream>
#include <cstdlib>
#include <vector>
#include <stack>
#include <sstream>
#include <list>
#include <fstream>
#include <typeinfo>
#include <algorithm>
#include <execution>
#include <math.h>

// the following code is created to create a binary tree strucutre for mathematical expressions in postfix noation as well as evaluating them with variables

using namespace std;

typedef string Elem;

class LinkedBinaryTree; // prototype

// copying the class from the textbook
class LinkedBinaryTree {
    protected:
        struct Node {
            Elem elt;
            Node * par;
            Node * left;
            Node * right;   
            Node() : par(NULL), left(NULL), right(NULL) {}
        };
    public:
        class Position { // position class to access protected variables of the node
            private:
                Node * v;
            public:
                Position(Node * _v = NULL) : v(_v) {}

                Elem& operator * () {
                    return v->elt;  
                }
                Position left() const { 
                    return Position(v->left); // returns the instance of the left node which we can then go in and use other methods
                }
                Position right() const { 
                    return Position(v->right); 
                }
                Position parent() const {
                    return Position(v->par); 
                }
                // added an accessor method
                Elem getelem() const {
                    return v->elt;
                }
                bool isRoot() const { 
                    return v->par == NULL; 
                }
                bool isExternal() const { 
                    return v->left == NULL && v->right == NULL; 
                }
                void removeright(const Position & p) {
                    delete (p.v -> right);
                    p.v -> right = NULL;
                }
                // added these additional methods to help reassign the left and right nodes
                void addleft(const Position & p) {
                    v->left = p.v;
                    v->left->par = v;
                }
                void addright(const Position & p) {
                    v->right = p.v;
                    v->right->par = v;
                }
                friend class LinkedBinaryTree;
        };
        typedef std::list<Position> PositionList; 
    public:
        LinkedBinaryTree();
        //~LinkedBinaryTree();
        void deletetree(Node * n);
        int size() const;
        bool empty() const;
        void printExpression();
        void printExpression(const Position& p);
        Position root() const;
        PositionList positions() const; // list of nodes
        void addRoot(); // add a root to an empty tre
        void setScore(double s);
        double getScore();
        void increasesize(int add) {
            n += add;
        }
        void expandExternal(const Position& p); // expand external node
        double evaluateExpression(double a, double b);
        double evaluateExpression(double a, double b, const Position& p);
        Position removeAboveExternal(const Position& p); // remove p and parent
        bool operator < (const LinkedBinaryTree& t2) const;
    protected:
        void preorder(Node * v, PositionList& pl) const;
    private:
        Node * _root; // pointer to the root
        int n; // number of nodes
        double score;
};

// methods
LinkedBinaryTree::LinkedBinaryTree() : _root(NULL), n(0), score(0) {} // constructor

/*LinkedBinaryTree::~LinkedBinaryTree() {
    if (_root != NULL) {
        deletetree(_root);
    }
}

// in addition, with the presence of the destructor method present, it outputs the trees in the incorrect order

void LinkedBinaryTree::deletetree(Node * n) { // destructor doesnt work as once you exit the createexpressiontree funciton, it deletes the returned tree
    if (n != NULL) { // the function does work for destroying the tree, but is called each time you fall out of the scope of a function, so after the createexpressiontree, it deletes all the trees so nothing is reutrned, so it doesnt work
        deletetree(n->left);
        deletetree(n->right);
        //delete n; this is commented out because it doesnt work with it
    }
}*/


int LinkedBinaryTree::size() const {
    return n;
}

bool LinkedBinaryTree::empty() const {
    return size() == 0;
}

LinkedBinaryTree::Position LinkedBinaryTree::root() const {
    return Position(_root);
}

void LinkedBinaryTree::addRoot() {
    _root = new Node;
    n = 1;
}

void LinkedBinaryTree::expandExternal(const Position& p) {
    Node * v = p.v; // p's node
    v->left = new Node;
    v->left->par = v;
    v->right = new Node;
    v->right->par = v;
    n += 2;
}

LinkedBinaryTree::Position LinkedBinaryTree::removeAboveExternal(const Position& p) {
    Node * w = p.v; 
    Node * v = w->par;
    Node * sib = (w == v->left ? v->right : v->left);
    if (v == _root) {
        _root = sib;
        sib -> par = NULL;
    } else {
        Node * gpar = v->par;
        if (v == gpar->left) gpar->left = sib;
        else gpar->right = sib;
        sib->par = gpar;
    }
    delete w; delete v;
    n -= 2;
    return Position(sib);
}

LinkedBinaryTree::PositionList LinkedBinaryTree::positions() const {
    PositionList pl;
    preorder(_root, pl);
    return PositionList(pl);
}

void LinkedBinaryTree::preorder(Node * v, PositionList& pl) const {
    pl.push_back(Position(v)); // store an instance of the class Position at node V;
    if (v->left != NULL) {
        preorder(v->left, pl);
    }
    if (v->right != NULL) {
        preorder(v->right, pl);
    }
}

void LinkedBinaryTree::printExpression() {
    printExpression(root());
}

void LinkedBinaryTree::printExpression(const Position& p) {
    if (p.isExternal()) { // if the node is external, then it will just print out the value stored there
        cout << p.getelem(); 
        return;
    } else {
        if (p.getelem() == "abs") { // if its an abs, then it will run a special set of code to print out properly and only access the left node
            cout << p.getelem();
            cout << "(";
            printExpression(p.left()); 
            cout << ")";
        } else { // code ran for other operators
            cout << "(";
            printExpression(p.left());
            cout << " " << p.getelem() << " ";
            printExpression(p.right());
            cout << ")";
        }
    }//*/
}

double LinkedBinaryTree::evaluateExpression(double a, double b) {
    return evaluateExpression(a,b,root());
}

double LinkedBinaryTree::evaluateExpression(double a, double b, const Position& p) {
    if (p.getelem() == "abs") { // special case for abs
        double _b;
        _b = evaluateExpression(a, b, p.left()); // only evalutes the expresison at the left node because the right node has nothing
        if (_b < 0) {
            return (-1) * _b; // returns the absolute value of _b
        } else {
            return _b;
        }
    } else if (!p.isExternal()) {
        double _a; double _b;
        _a = evaluateExpression(a, b, p.left()); // recursively call the function for the left and right node
        _b = evaluateExpression(a, b, p.right());
        // if statement for each operator and return the correct operation/operand combination
        double result;
        if (p.getelem() == "*") {
            result = _a * _b;
        } else if (p.getelem() == "+") {
            result = _a + _b;
        } else if (p.getelem() == "-") {
            result = _a - _b;
        } else if (p.getelem() == "/") {
            result = _a / _b;
        } else if (p.getelem() == ">") {
            if (_a > _b) {
                return 1;
            } else {
                return -1;
            }
        }
        return isnan(result) || !isfinite(result) ? 0 : result;
    }
    // substitutes values for a and b or converts the number from string to double and return it if it isnt an a or b
    if (p.getelem() == "a") {
        return a;
    } else if (p.getelem() == "b") {
        return b;
    } else {
        return stod(p.getelem());
    }
}

void LinkedBinaryTree::setScore(double s) {
    score = s; // sets the score
}

double LinkedBinaryTree::getScore() {
    return score; // accessor method to return the score
}

bool LinkedBinaryTree::operator < (const LinkedBinaryTree& t2) const { // < operator overload to compare the score of the two trees
    return (score) < (t2.score); // compares the score of the two trees
}

// helper funciton
LinkedBinaryTree createExpressionTree(string postfix) {
    std::stack<LinkedBinaryTree> s; // uses the stack library to create a stack S of type LinkedBinaryTree that will store instances of the class
    // tokenizing the output
    string tok;
    stringstream ss(postfix);
    while (getline(ss, tok, ' ')) { // tokenizes the string with the space delimiter
        if (tok == "*" || tok == "+" || tok == "-" || tok == "/" || tok == ">") { // set of code for regular opertions
            LinkedBinaryTree t; // creates a LinkedBinaryTree t
            t.addRoot(); // adds a root
            *(t.root()) = tok; // assigns the roots element to be the token

            // saving the tress in the stack and popping them out
            LinkedBinaryTree right = s.top(); 
            s.pop();
            LinkedBinaryTree left = s.top();
            s.pop();

            t.expandExternal(t.root()); // expands the root to have a left and right child node
            t.increasesize(right.size() + left.size() - 2);
            // adding the new nodes
            t.root().addleft(left.root()); // assigned the left and right child nodes to be the roots of trees that were popped out of the stack
            t.root().addright(right.root());

            s.push(t); // pushes it back onto the stack
    
        } else if (tok == "abs") { // case for abs
            LinkedBinaryTree t;
            t.addRoot();
            *(t.root()) = tok;

            LinkedBinaryTree left = s.top(); // only pop out 1 item since as only takes in 1 input
            s.pop();

            t.expandExternal(t.root());
            t.root().removeright(t.root());
            t.increasesize(left.size() - 2);

            t.root().addleft(left.root()); // only assignst he left child of the root

            s.push(t);
        } else { // if the token is a, b or a number
            LinkedBinaryTree t;
            t.addRoot();
            *(t.root()) = tok;
            s.push(t);
        }
        
    }
    LinkedBinaryTree final = s.top(); // the final tree is the top item of the stack
    s.pop(); // pops out the stack
    return final; // returns the final tree
}

/* MAIN FUNCTIOn */

int main() {
    // Read postfix expressions into vector of expression trees
    vector<LinkedBinaryTree> trees;
    ifstream exp_file("expressions.txt");
    string line;
    while (getline(exp_file, line)) {
        trees.push_back(createExpressionTree(line));
    }

    // Read input data into 2D vector of doubles
    vector<vector<double> > inputs;
    ifstream input_file("input.txt");
    while (getline(input_file, line)) {
        vector<double> ab_input;
        stringstream ss(line);
        string str;
        while (getline(ss, str, ' ')) {
            ab_input.push_back(stod(str));
        }  
        inputs.push_back(ab_input);
    }

    // Evaluate expressions on input data
    for (auto& t : trees) {
        double sum = 0;
        for (auto& i : inputs) {
            sum += t.evaluateExpression(i[0], i[1]); // passing in the root position to make it recursive
        }
        t.setScore(sum/inputs.size());
    }

    // Sort trees by their score
    sort(trees.begin(), trees.end());
    for (auto& t : trees) {
        cout << "Exp ";
        t.printExpression(); // pasing in arguments to make this recursive
        cout << " Score " << t.getScore() << endl;
    }
    return 0;
}

