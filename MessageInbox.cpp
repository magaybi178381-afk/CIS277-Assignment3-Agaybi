#include "MessageInbox.h"

MessageInbox::MessageInbox(std::size_t maxTextBytes)
    : head_(nullptr), tail_(nullptr), count_(0), usedBytes_(0),
      maxBytes_(maxTextBytes) {}

MessageInbox::~MessageInbox() {
    clear();
}

MessageInbox::Node* MessageInbox::findNode(std::size_t id) const {
    for (Node* cur = head_; cur != nullptr; cur = cur->next) {
        if (cur->message.id == id) {
            return cur;
        }
    }
    return nullptr;
}

bool MessageInbox::receive(std::size_t id,
                           const std::string& sender,
                           const std::string& text) {
    // Validate everything first so a rejected arrival changes nothing.
    if (id == 0 || sender.empty() || text.empty()) return false;
    if (text.size() > remainingTextBytes()) return false;
    if (findNode(id) != nullptr) return false;

    // Fully initialize the node before linking it.
    Node* node = new Node(id, sender, text);

    if (tail_ == nullptr) {   
        head_ = node;
    } else {
        tail_->next = node;
    }
    tail_ = node;                

    ++count_;
    usedBytes_ += text.size();
    return true;
}

bool MessageInbox::read(std::size_t id, TextMessage& result) {
    Node* node = findNode(id);
    if (node == nullptr) return false;  
    node->message.isRead = true;
    result = node->message;
    return true;
}

bool MessageInbox::erase(std::size_t id) {
    Node* prev = nullptr;
    Node* cur = head_;
    while (cur != nullptr && cur->message.id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == nullptr) return false;

    // Reconnect the list before deleting.
    if (prev == nullptr) {
        head_ = cur->next;       
    } else {
        prev->next = cur->next;
    }
    if (cur == tail_) {
        tail_ = prev;            
    }

    --count_;
    usedBytes_ -= cur->message.text.size();
    delete cur;
    return true;
}

std::vector<TextMessage> MessageInbox::messages() const {
    std::vector<TextMessage> copies;
    copies.reserve(count_);
    for (Node* cur = head_; cur != nullptr; cur = cur->next) {
        copies.push_back(cur->message);
    }
    return copies;
}

void MessageInbox::clear() {
    Node* cur = head_;
    while (cur != nullptr) {
        Node* next = cur->next;  // save before deleting
        delete cur;
        cur = next;
    }
    head_ = nullptr;
    tail_ = nullptr;
    count_ = 0;
    usedBytes_ = 0;
}

std::size_t MessageInbox::messageCount() const { return count_; }
std::size_t MessageInbox::usedTextBytes() const { return usedBytes_; }
std::size_t MessageInbox::remainingTextBytes() const { return maxBytes_ - usedBytes_; }
