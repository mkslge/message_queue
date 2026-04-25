#include "Payload.h"
#include "Node.h"


#include <thread>
#include <mutex>
#include <memory>
#include <condition_variable>

class Queue {
    Node* dummy_head_;
    Node* dummy_tail_;

    std::mutex head_mutex_;
    std::mutex tail_mutex_;
    int size_;
    std::condition_variable cv_;

    public:
    Queue() : size_(0), dummy_head_(new Node(0)), dummy_tail_(new Node(0)) {
        dummy_tail_->next_ = dummy_head_;
        dummy_head_->prev_ = dummy_tail_;
        
        dummy_tail_->prev_ = nullptr;
        dummy_head_->next_ = nullptr;
    }

    void enqueue(Payload& pl) {
        std::lock_guard lg(head_mutex_);
        Node* tail= new Node(pl);
        tail->next_ = dummy_tail_->next_;
        tail->prev_ = dummy_tail_;
        
        dummy_tail_->next_->prev_ = tail;
        dummy_tail_->next_ = tail;

        size_++;
        cv_.notify_all();
    }

    Payload dequeue() {
        std::unique_lock<std::mutex> ul(head_mutex_);
        //release lock and sleep until the queue is not empty
        cv_.wait(ul, [this]{return size_ > 0;});
        Node* to_delete = dummy_head_->prev_;
        Payload to_ret = to_delete->val_;
        to_delete->prev_->next_ = to_delete->next_;
        to_delete->next_->prev_ = to_delete->prev_;
        size_--;
        ul.unlock();

        delete to_delete;
        
        

        return to_ret;
        
    }




    //rule of 3
    Queue(Queue& other) {
        Node* curr = dummy_tail_->next_;
        for(auto i = 0; i < size_;i++) {
            Node* to_del = curr;
            curr = to_del->next_;

            delete to_del;
        }
        size_ = 0;
        delete dummy_head_;
        delete dummy_tail_;

        dummy_head_ = new Node(0);
        dummy_tail_ = new Node(0);
        dummy_tail_->next_ = dummy_head_;
        dummy_head_->prev_ = dummy_tail_;

        curr = dummy_head_;
        Node* curr_other = other.dummy_head_;
        for(int i = 0; i < other.size_;i++) {
            curr_other = curr_other->prev_;
            Node* copy = new Node(curr_other->val_);
            copy->next_ = curr;
            copy->prev_ = curr->prev_;
            curr->prev_->next_ = copy;
            curr->prev_ = copy;

            curr = copy;
        }


    }

    Queue& operator=(Queue& other) = delete;

    ~Queue() {
        Node* curr = dummy_tail_;
        while(curr != nullptr) {
            Node* delete_me = curr;
            curr = curr->next_;
            delete delete_me;
        }
        size_ = 0;
    }



};
