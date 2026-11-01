#include <iostream>
#include "MessageInbox.h"

static void show(const MessageInbox& inbox, const std::string& title) {
    std::cout << "--- " << title << " ---\n";
    for (const TextMessage& m : inbox.messages()) {
        std::cout << "  [" << m.id << "] " << m.sender << ": \"" << m.text
                  << "\" (" << (m.isRead ? "read" : "unread") << ")\n";
    }
    std::cout << "  messages=" << inbox.messageCount()
              << " used=" << inbox.usedTextBytes()
              << " remaining=" << inbox.remainingTextBytes() << "\n\n";
}

static void report(const char* action, bool ok) {
    std::cout << action << " -> " << (ok ? "accepted/true" : "rejected/false") << "\n";
}

int main() {
    MessageInbox inbox(20);

    report("receive 101 Maya \"Hi!\"", inbox.receive(101, "Maya", "Hi!"));
    report("receive 102 Noah \"Meet at 6\"", inbox.receive(102, "Noah", "Meet at 6"));
    report("receive 103 Eli \"Bring notes\" (over budget)",
           inbox.receive(103, "Eli", "Bring notes"));
    show(inbox, "After first three arrivals");

    TextMessage out{0, "", "", false};
    report("read 101", inbox.read(101, out));
    std::cout << "  returned: " << out.sender << ": " << out.text
              << " (" << (out.isRead ? "read" : "unread") << ")\n";
    show(inbox, "After reading 101 (still stored, bytes unchanged)");

    report("erase 101", inbox.erase(101));
    show(inbox, "After deleting 101");

    report("receive 103 Eli \"Bring notes\" (again)",
           inbox.receive(103, "Eli", "Bring notes"));
    show(inbox, "After accepting 103 (exactly 20 bytes)");

    report("read 999 (missing)", inbox.read(999, out));
    report("erase 999 (missing)", inbox.erase(999));

    inbox.clear();
    show(inbox, "After clear()");
    return 0;
}
