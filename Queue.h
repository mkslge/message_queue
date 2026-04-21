#include "Payload.h"
#include "Node.h"


#include <thread>
#include <mutex>
#include <memory>

class Queue {
    Node* dummy_head_;
    Node* dummy_tail_;

    std::mutex head_mutex_;
    std::mutex tail_mutex_;
    int size_;


    
    Queue() : size_(0), dummy_head_(new Node(0)), dummy_tail_(new Node(0)) {
        dummy_tail_->next_ = dummy_head_;
        dummy_head_->prev_ = dummy_tail_;
        
        dummy_tail_->prev_ = nullptr;
        dummy_head_->next_ = nullptr;
    }

    void enqueue(Payload& pl) {
        std::lock_guard lg(tail_mutex_);
        Node* tail= new Node(pl);
        tail->next_ = dummy_tail_->next_;
        tail->prev_ = dummy_tail_;
        
        dummy_tail_->next_->prev_ = tail;
        dummy_tail_->next_ = tail;

        size_++;
    }

    Payload dequeue() {
        //sleep while dequeue is false;

        std::lock_guard lg(head_mutex_);
        Node* to_delete = dummy_head_->prev_;
        Payload to_return = to_delete->val_;
        
        to_delete->prev_->next_ = to_delete->next_;
        to_delete->next_->prev_ = to_delete->prev_;

        delete to_delete;
        size_--;

        return to_return;
    }




    //rule of 3
    Queue(Queue& other)  : dummy_head_(other.dummy_head_),
    dummy_tail_(other.dummy_tail_),
    size_(other.size_)
    {}

    Queue& operator=(Queue& other) {
        std::lock_guard lgh(head_mutex_);
        std::lock_guard lgt(tail_mutex_);

        std::swap(dummy_head_, other.dummy_head_);
        std::swap(dummy_tail_, other.dummy_tail_);
        size_ = other.size_;

        return *this;
    }
    ~Queue() {
        Node* curr = dummy_tail_->next_;
        for(auto i = 0; i < size_;i++) {
            Node* to_del = curr;
            curr = to_del->next_;

            delete to_del;
        }


        size_ = 0;
        delete dummy_head_;
        delete dummy_tail_;
    }



};


