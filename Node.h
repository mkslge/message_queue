#include "Payload.h"

class Node {
    public:

        Node* next_;
        Node* prev_;
        Payload val_;
        Node(const Payload& val) : val_(val) {}



};