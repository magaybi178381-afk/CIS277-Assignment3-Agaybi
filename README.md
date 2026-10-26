**CIS-277 Assignment 3: Text Messages and Dynamic Memory
Student: Marc Agaybi**

Node fields: a private Node holding a TextMessage message and Node* next. Constructor sets isRead = false and next = nullptr.
Ownership: the MessageInbox owns every node. Nodes are created with new in receive and released with delete in erase and clear. The destructor calls clear(). Copying is deleted so two inboxes can never own the same chain.
Head/tail updates: receive links at the tail (tail_->next = node; tail_ = node, or sets both head and tail if empty). erase walks with a prev pointer; deleting the head moves head_, deleting the tail moves tail_ to prev (null if it was the only node).
Byte count: usedBytes_ increases by text.size() on accepted arrival and decreases by the same amount on erase. clear() resets it to 0. Accessors only return fields.
Order of checks in receive: validate id/sender/text, budget, and duplicate id before allocating, so rejections change nothing.

**How to Compile**

g++ -std=c++23 -Wall -Wextra -pedantic main.cpp MessageInbox.cpp -o message_inbox

**How to Run**

./message_inbox
python validate.py

g++ -std=c++23 -Wall -Wextra -pedantic -I. tests/my_tests.cpp MessageInbox.cpp -o my_tests && ./my_tests

**Tests and Results**

tests/my_tests.cpp prints expected vs. actual for each check. All 38 checks passed, with no corrections needed in the final run. Compiled with -fsanitize=address,undefined (AddressSanitizer includes LeakSanitizer): no errors and no leaks reported. Valgrind was not available in my environment.

Area	Expected	Actual
Empty read / erase / clear (twice)	false / false / safe, count 0	as expected

ID 0, empty sender, empty text, duplicate ID	rejected, state unchanged	as expected

Text of exactly remaining bytes	accepted, remaining 0	as expected

One byte over budget	rejected	as expected

Missing-ID read	false, output untouched	as expected

Read twice	true both times, used bytes unchanged	as expected

Delete head, middle, tail, only node	order 2,3,4 → 2,4 → 2 → empty	as expected

Append after singleton/tail delete	new node is the tail, order correct	as expected

Reuse a deleted ID	accepted, appended at end	as expected

Modify snapshot copies	stored messages unchanged	as expected

Reuse after clear()	full budget restored, receive works	as expected


The 20-byte demo in main.cpp reproduces the assignment table (3 → 12 → rejected → read → 9 → 20 bytes).

**Pointer diagrams**

Delete first (101) from 101 -> 102 -> 103:

Before: head -> [101] -> [102] -> [103] -> null     tail -> [103]
After:  head -> [102] -> [103] -> null              tail -> [103]
Changed: head. Node 101 deleted. tail unchanged.

Delete last (103):

Before: head -> [101] -> [102] -> [103] -> null     tail -> [103]
After:  head -> [101] -> [102] -> null              tail -> [102]
Changed: [102].next = null, tail. Node 103 deleted. head unchanged.

Delete only node:

Before: head -> [101] -> null     tail -> [101]
After:  head = null               tail = null
Changed: head and tail both become null.

**Analysis Questions**

Pointers on delete. First: head moves to the next node (tail unchanged unless it was the only node). Last: the previous node's next becomes null and tail moves to that previous node. Only node: both head and tail become null. See diagrams above.

Read vs. delete. Reading copies the message and sets isRead; the node and its text stay stored, so the bytes are still in use. Deleting removes the text from the inbox, so its text.size() is subtracted from the counter and that room can be reused.

What the counter measures. Only the sum of text.size() over stored messages. It excludes sender strings, node fields (next, id, isRead), std::string capacity and small-string storage, allocator overhead, and the null terminator. It models a text-storage limit, not real RAM use.

Why receive is O(n). Appending at the tail is O(1) link changes because tail_ is kept, but the duplicate-ID check must traverse up to n nodes. Allocating the node and copying sender and text costs time proportional to their lengths (plus the allocator's cost), which is separate from the list search.

Ownership problems. Nodes left allocated after destruction are leaked memory. Two inboxes sharing one chain would each try to delete the same nodes (double free / use after free), and a change in one would corrupt the other's head, tail, and counts. Deleting copy operations prevents this.


**AI Use:**
I used Claude to generate a first draft of the code and tests, then reviewed every line.
