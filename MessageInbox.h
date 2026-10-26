#ifndef MESSAGE_INBOX_H
#define MESSAGE_INBOX_H
 
#include <cstddef>
#include <string>
#include <vector>

struct TextMessage {
    std::size_t id;
    std::string sender;
    std::string text;
    bool isRead;
};

class MessageInbox {
public:
    explicit MessageInbox(std::size_t maxTextBytes);
    ~MessageInbox();
    MessageInbox(const MessageInbox&) = delete;
    MessageInbox& operator=(const MessageInbox&) = delete;
 
    bool receive(std::size_t id,
                 const std::string& sender,
                 const std::string& text);
    bool read(std::size_t id, TextMessage& result);
    bool erase(std::size_t id);
    std::vector<TextMessage> messages() const;
    void clear();
    std::size_t messageCount() const;
    std::size_t usedTextBytes() const;
    std::size_t remainingTextBytes() const;

private:

    struct Node {
        TextMessage message;
        Node* next;
        Node(std::size_t id, const std::string& sender, const std::string& text)
            : message{id, sender, text, false}, next(nullptr) {}
    };

    Node* findNode(std::size_t id) const;

    Node* head_;
    Node* tail_;
    std::size_t count_;
    std::size_t usedBytes_;
    std::size_t maxBytes_;
};
