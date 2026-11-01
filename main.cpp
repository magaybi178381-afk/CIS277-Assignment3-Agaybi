#include <iostream>
#include <string>
#include "MessageInbox.h"

static int failures = 0;
template <class T>
static void check(const std::string& name, const T& expected, const T& actual) {
    bool ok = (expected == actual);
    std::cout << (ok ? "PASS " : "FAIL ") << name << " | expected=" << expected
              << " actual=" << actual << "\n";
    if (!ok) ++failures;
}
static std::string ids(const MessageInbox& in) {
    std::string s;
    for (auto& m : in.messages()) s += std::to_string(m.id) + ",";
    return s;
}

int main() {
    { 
        MessageInbox in(10);
        TextMessage out{7, "x", "y", false};
        check("empty read false", false, in.read(1, out));
        check("empty read leaves output", std::string("y"), out.text);
        check("empty erase false", false, in.erase(1));
        check("empty count", std::size_t(0), in.messageCount());
        check("empty remaining", std::size_t(10), in.remainingTextBytes());
        in.clear(); in.clear();
        check("clear on empty ok", std::size_t(0), in.messageCount());
    }
    { 
        MessageInbox in(10);
        check("id 0 rejected", false, in.receive(0, "A", "hi"));
        check("empty sender rejected", false, in.receive(1, "", "hi"));
        check("empty text rejected", false, in.receive(1, "A", ""));
        check("valid accepted", true, in.receive(1, "A", "hi"));
        check("duplicate rejected", false, in.receive(1, "B", "yo"));
        check("state unchanged", std::size_t(2), in.usedTextBytes());
        check("count unchanged", std::size_t(1), in.messageCount());
    }
    { 
        MessageInbox in(5);
        check("exact budget accepted", true, in.receive(1, "A", "12345"));
        check("remaining 0", std::size_t(0), in.remainingTextBytes());
        check("over budget rejected", false, in.receive(2, "A", "x"));
        in.erase(1);
        check("space reused", true, in.receive(1, "A", "abcde"));
        MessageInbox in2(5);
        check("6 bytes rejected", false, in2.receive(1, "A", "123456"));
    }
    { 
        MessageInbox in(20);
        in.receive(1, "A", "abc");
        TextMessage out{};
        check("read true", true, in.read(1, out));
        check("read flag", true, out.isRead);
        check("read twice true", true, in.read(1, out));
        check("budget unchanged by read", std::size_t(3), in.usedTextBytes());
        check("stored read", true, in.messages()[0].isRead);
    }
    { 
        MessageInbox in(100);
        for (std::size_t i = 1; i <= 4; ++i) in.receive(i, "S", "m");
        in.erase(1); check("delete head", std::string("2,3,4,"), ids(in));
        in.erase(3); check("delete middle", std::string("2,4,"), ids(in));
        in.erase(4); check("delete tail", std::string("2,"), ids(in));
        in.receive(5, "S", "m"); check("append after tail delete", std::string("2,5,"), ids(in));
        in.erase(2); in.erase(5);
        check("only node deleted count", std::size_t(0), in.messageCount());
        in.receive(6, "S", "m"); check("append after empty", std::string("6,"), ids(in));
        in.erase(6); // singleton delete then append
        in.receive(7, "S", "m"); in.receive(8, "S", "m");
        check("append after singleton delete", std::string("7,8,"), ids(in));
        check("missing erase false", false, in.erase(99));
        check("id reuse", true, in.erase(7) && in.receive(7, "S", "m"));
        check("order after reuse", std::string("8,7,"), ids(in));
    }
    { 
        MessageInbox in(20);
        in.receive(1, "A", "abc");
        auto snap = in.messages();
        snap[0].text = "changed"; snap[0].isRead = true;
        check("snapshot independent text", std::string("abc"), in.messages()[0].text);
        check("snapshot independent read", false, in.messages()[0].isRead);
    }
    { 
        MessageInbox in(10);
        in.receive(1, "A", "abcde"); in.receive(2, "A", "abc");
        in.clear(); in.clear();
        check("clear count", std::size_t(0), in.messageCount());
        check("clear remaining", std::size_t(10), in.remainingTextBytes());
        check("reuse after clear", true, in.receive(1, "A", "0123456789"));
    }
    std::cout << (failures == 0 ? "ALL PASSED" : "SOME FAILED") << "\n";
    return failures;
}
