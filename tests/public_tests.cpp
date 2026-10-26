// Public CIS-277 Assignment 3 checks. No student implementation is supplied.
// Include MessageInbox.h first: it must supply its own required includes.
#include "MessageInbox.h"

#if __cplusplus < 202100L
#error "Assignment 3 requires a C++23-capable compiler and C++23 mode"
#endif

#include <cstddef>
#include <type_traits>

using Inbox = MessageInbox;
static_assert(std::is_same_v<decltype(TextMessage::id), std::size_t>, "TextMessage id type");
static_assert(std::is_same_v<decltype(TextMessage::sender), std::string>, "TextMessage sender type");
static_assert(std::is_same_v<decltype(TextMessage::text), std::string>, "TextMessage text type");
static_assert(std::is_same_v<decltype(TextMessage::isRead), bool>, "TextMessage isRead type");
static_assert(std::is_constructible_v<Inbox, std::size_t>, "MessageInbox constructor signature");
static_assert(std::is_destructible_v<Inbox>, "MessageInbox destructor");
static_assert(!std::is_copy_constructible_v<Inbox>, "inbox copy construction must be deleted");
static_assert(!std::is_copy_assignable_v<Inbox>, "inbox copy assignment must be deleted");
static_assert(!std::is_move_constructible_v<Inbox>, "inbox must not be movable");
static_assert(!std::is_move_assignable_v<Inbox>, "inbox must not be movable");
static_assert(std::is_same_v<decltype(static_cast<bool (Inbox::*)(std::size_t, const std::string&, const std::string&)>(&Inbox::receive)), bool (Inbox::*)(std::size_t, const std::string&, const std::string&)>, "receive signature");
static_assert(std::is_same_v<decltype(static_cast<bool (Inbox::*)(std::size_t, TextMessage&)>(&Inbox::read)), bool (Inbox::*)(std::size_t, TextMessage&)>, "read signature");
static_assert(std::is_same_v<decltype(static_cast<bool (Inbox::*)(std::size_t)>(&Inbox::erase)), bool (Inbox::*)(std::size_t)>, "erase signature");
static_assert(std::is_same_v<decltype(static_cast<std::vector<TextMessage> (Inbox::*)() const>(&Inbox::messages)), std::vector<TextMessage> (Inbox::*)() const>, "messages signature");
static_assert(std::is_same_v<decltype(static_cast<void (Inbox::*)()>(&Inbox::clear)), void (Inbox::*)()>, "clear signature");
static_assert(std::is_same_v<decltype(static_cast<std::size_t (Inbox::*)() const>(&Inbox::messageCount)), std::size_t (Inbox::*)() const>, "messageCount signature");
static_assert(std::is_same_v<decltype(static_cast<std::size_t (Inbox::*)() const>(&Inbox::usedTextBytes)), std::size_t (Inbox::*)() const>, "usedTextBytes signature");
static_assert(std::is_same_v<decltype(static_cast<std::size_t (Inbox::*)() const>(&Inbox::remainingTextBytes)), std::size_t (Inbox::*)() const>, "remainingTextBytes signature");

#ifdef A3_HEADER_ONLY
// No constructor or method is called; this checks declarations without linking
// MessageInbox.cpp. Full implementation behavior is checked in the next stage.
int main() { return 0; }
#else

#include <cstdlib>
#include <initializer_list>
#include <iostream>
#include <string>
#include <vector>

std::size_t checks = 0;

void require(bool condition, const char* label) {
    ++checks;
    if (!condition) {
        std::cerr << "FAIL: " << label << " (check " << checks << ")\n";
        std::exit(1);
    }
}

bool same_message(const TextMessage& left, const TextMessage& right) {
    return left.id == right.id && left.sender == right.sender &&
           left.text == right.text && left.isRead == right.isRead;
}

void expect_inbox(const Inbox& inbox, std::size_t budget,
                  std::initializer_list<TextMessage> expected, const char* label) {
    const auto snapshot = inbox.messages();
    std::size_t text_bytes = 0;
    for (const auto& message : expected) {
        text_bytes += message.text.size();
    }
    require(inbox.messageCount() == expected.size(), label);
    require(inbox.usedTextBytes() == text_bytes, label);
    require(inbox.remainingTextBytes() == budget - text_bytes, label);
    require(snapshot.size() == expected.size(), label);
    std::size_t index = 0;
    for (const auto& message : expected) {
        require(same_message(snapshot[index++], message), label);
    }
}

void test_budget_example_and_read() {
    Inbox inbox(20);
    TextMessage result{777, "keep sender", "keep text", false};
    const TextMessage unchanged = result;
    expect_inbox(inbox, 20, {}, "new inbox has its full text-byte budget");
    require(!inbox.read(999, result) && same_message(result, unchanged), "empty read preserves result");
    require(!inbox.erase(999), "empty erase returns false");
    require(!inbox.receive(0, "Maya", "Hi!"), "ID zero is rejected");
    expect_inbox(inbox, 20, {}, "ID zero leaves state unchanged");
    require(!inbox.receive(101, "", "Hi!"), "empty sender is rejected");
    expect_inbox(inbox, 20, {}, "empty sender leaves state unchanged");
    require(!inbox.receive(101, "Maya", ""), "empty text is rejected");
    expect_inbox(inbox, 20, {}, "empty text leaves state unchanged");
    require(!inbox.receive(101, "Maya", std::string(21, 'x')), "text larger than total budget is rejected");
    expect_inbox(inbox, 20, {}, "oversize text leaves empty state unchanged");

    require(inbox.receive(101, "Maya", "Hi!"), "receive Maya's three-byte text");
    require(inbox.receive(102, "Noah", "Meet at 6"), "receive Noah's nine-byte text");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", false}}, "arrival order and twelve text bytes used");
    require(!inbox.receive(103, "Eli", "Bring notes"), "eleven-byte text cannot fit eight remaining bytes");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", false}}, "budget rejection leaves both messages unchanged");
    require(!inbox.receive(101, "Other sender", "x"), "currently stored duplicate ID is rejected");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", false}}, "duplicate rejection leaves state unchanged");

    require(inbox.read(102, result), "read existing message succeeds");
    require(same_message(result, {102, "Noah", "Meet at 6", true}), "read returns the message marked read");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", true}}, "read updates one flag without freeing text bytes");
    require(inbox.read(102, result), "repeated read succeeds");
    require(same_message(result, {102, "Noah", "Meet at 6", true}), "repeated read returns same marked-read message");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", true}}, "repeated read keeps arrival order and text budget");
    result = unchanged;
    require(!inbox.read(0, result) && same_message(result, unchanged), "missing ID zero preserves read result");
    require(!inbox.read(999, result) && same_message(result, unchanged), "missing ID preserves read result");
    require(!inbox.erase(999), "missing ID erase returns false");
    expect_inbox(inbox, 20, {{101, "Maya", "Hi!", false}, {102, "Noah", "Meet at 6", true}}, "missing-ID operations preserve messages and flags");

    require(inbox.erase(101), "erase Maya's message reclaims three text bytes");
    expect_inbox(inbox, 20, {{102, "Noah", "Meet at 6", true}}, "eleven bytes available after erase");
    require(inbox.receive(103, "Eli", "Bring notes"), "Eli's message fits reclaimed budget exactly");
    expect_inbox(inbox, 20, {{102, "Noah", "Meet at 6", true}, {103, "Eli", "Bring notes", false}}, "exact twenty-byte budget and appended arrival order");
    require(!inbox.receive(104, "Tess", "x"), "one extra text byte is rejected at full budget");
    expect_inbox(inbox, 20, {{102, "Noah", "Meet at 6", true}, {103, "Eli", "Bring notes", false}}, "full-budget rejection preserves state");
    require(inbox.erase(103), "erase tail reclaims eleven text bytes");
    require(inbox.receive(101, "Maya", "Bye"), "deleted ID may be reused with a new unread message");
    expect_inbox(inbox, 20, {{102, "Noah", "Meet at 6", true}, {101, "Maya", "Bye", false}}, "reused ID appears last and starts unread");
}

void test_delete_locations_and_reuse() {
    Inbox inbox(30);
    require(inbox.receive(1, "A", "a") && inbox.receive(2, "B", "bb") &&
            inbox.receive(3, "C", "ccc") && inbox.receive(4, "D", "dddd") &&
            inbox.receive(5, "E", "eeeee"), "create five messages");
    require(inbox.erase(3), "erase middle node");
    expect_inbox(inbox, 30, {{1, "A", "a", false}, {2, "B", "bb", false}, {4, "D", "dddd", false}, {5, "E", "eeeee", false}}, "middle deletion reconnects neighbors and reclaims text bytes");
    require(inbox.erase(5), "erase tail node");
    require(inbox.receive(6, "F", "ffffff"), "append after tail deletion");
    expect_inbox(inbox, 30, {{1, "A", "a", false}, {2, "B", "bb", false}, {4, "D", "dddd", false}, {6, "F", "ffffff", false}}, "tail remains correct for next append");
    require(inbox.erase(1), "erase head node");
    expect_inbox(inbox, 30, {{2, "B", "bb", false}, {4, "D", "dddd", false}, {6, "F", "ffffff", false}}, "head deletion preserves remaining order");
    require(inbox.erase(6), "erase new tail node");
    require(inbox.erase(2), "erase head until one message remains");
    expect_inbox(inbox, 30, {{4, "D", "dddd", false}}, "singleton state after multiple deletions");
    require(inbox.erase(4), "erase singleton node");
    expect_inbox(inbox, 30, {}, "singleton deletion restores full budget");
    require(!inbox.erase(4) && !inbox.erase(0), "missing erase after drain returns false");
    require(inbox.receive(7, "Long sender name not charged to text budget", std::string(30, 'z')), "only text bytes count against budget");
    expect_inbox(inbox, 30, {{7, "Long sender name not charged to text budget", std::string(30, 'z'), false}}, "append after singleton deletion can use entire budget");
}

void test_copies_clear_and_lifetimes() {
    Inbox inbox(12);
    std::string sender = "Alice";
    std::string text = "Original";
    require(inbox.receive(11, sender, text), "receive stores copied text and sender");
    sender = "Changed sender";
    text = "Changed text";
    expect_inbox(inbox, 12, {{11, "Alice", "Original", false}}, "later changes to inputs do not alter stored message");
    auto copy = inbox.messages();
    require(copy.size() == 1, "snapshot returns stored message");
    copy[0].id = 999;
    copy[0].sender = "Edited";
    copy[0].text = "Edited text";
    copy[0].isRead = true;
    copy.clear();
    expect_inbox(inbox, 12, {{11, "Alice", "Original", false}}, "modifying and clearing returned snapshot leaves inbox unchanged");
    TextMessage result{0, "", "", false};
    require(inbox.read(11, result), "read for copy-ownership check");
    result.id = 777;
    result.sender = "Edited output";
    result.text.clear();
    result.isRead = false;
    expect_inbox(inbox, 12, {{11, "Alice", "Original", true}}, "modifying read result does not alter stored message");
    require(inbox.receive(12, "Bob", "four"), "fill remaining four-byte budget");
    inbox.clear();
    expect_inbox(inbox, 12, {}, "clear deletes all messages and restores text budget");
    inbox.clear();
    expect_inbox(inbox, 12, {}, "clear on empty inbox is idempotent");
    result = {777, "keep", "untouched", true};
    const TextMessage unchanged = result;
    require(!inbox.read(11, result) && same_message(result, unchanged), "read after clear preserves result");
    require(inbox.receive(11, "Alice", std::string(12, 'a')), "reuse entire text budget and old ID after clear");
    expect_inbox(inbox, 12, {{11, "Alice", std::string(12, 'a'), false}}, "reused inbox has fresh unread message");
    inbox.clear();
    expect_inbox(inbox, 12, {}, "second populated clear restores all budget");

    for (std::size_t repetition = 1; repetition <= 8; ++repetition) {
        Inbox populated(10);
        require(populated.receive(repetition, "One", "abc") &&
                populated.receive(repetition + 100, "Two", "defg"), "populate inbox before destruction");
        TextMessage read_result{0, "", "", false};
        require(populated.read(repetition, read_result) && read_result.isRead,
                "read before destroying populated inbox");
        require(populated.messageCount() == 2 && populated.usedTextBytes() == 7,
                "populated destructor test retains two nodes");
        // Scope exit must free both live nodes; sanitizer mode checks memory use.
    }
}

int main() {
    test_budget_example_and_read();
    test_delete_locations_and_reuse();
    test_copies_clear_and_lifetimes();
    std::cout << "PASS: " << checks << " runtime checks: text-byte budget, arrival order, read flags, copied snapshots, deletion, clear, and reuse.\n";
}
#endif
