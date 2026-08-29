# Browser History Manager

A CLI-based application built in C++ that simulates a web browser's history and bookmark management system. 

This project was developed to practice foundational computer science concepts—specifically **custom data structures**, **manual memory management**, and **pointer manipulation**—without relying on C++ standard library containers like `std::vector` or `std::list`.

## Features
* **History Tracking:** Log visited pages with auto-generated IDs and timestamps.
* **Bookmark Management:** Add pages to bookmarks, mark as favorites, track visit counts, and auto-remove the least visited bookmarks.
* **Related Pages:** Link associated URLs to a main webpage using a nested linked list structure.
* **Search & Filter:** Search history by URL substring or filter and delete pages older than a specific date.
* **Persistent Storage:** Save and load history, bookmarks, and related pages from a formatted text file (`input.txt`).

## Under the Hood: Data Structures
Instead of standard containers, this project utilizes custom-built data structures to manage memory directly:
* **Doubly Linked List (Browser History):** Allows efficient traversal forward and backward through chronologically visited pages.
* **Singly Linked List (Related Pages):** Nested inside the History nodes to maintain a lightweight list of sub-pages or associated links.
* **Doubly Linked List (Bookmarks):** Maintains user favorites with O(1) node deletion and tracks individual node visit counts.

## Getting Started

### Prerequisites
* A standard C++ compiler (e.g., GCC, Clang, or MSVC).

### Compilation and Execution
To compile the project from your terminal, run:

```bash
g++ main.cpp -o browser_history
