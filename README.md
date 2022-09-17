# C++ PRIMER

> [fall2022: project 0](https://15445.courses.cs.cmu.edu/fall2022/project0/)

## TEMPLATED TRIE

Before starting, read the debug info in *src/include/common/logger.h*.
* ERROR > WARN > INFO > DEBUG > TRACE (Refers to
  [When to use different log levels](https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels))

### TrieNode Class

* `explicit`: Mark the constructor cannot be implicitly conversion or copy
  initialization
* move constructor: use a rvalue reference `TrieNode &&` to transfer the
  unique_ptr to a new TrieNode

Others are normal stuff.

### TrieNodeWithValue Class

Use an initialize list to init base members.

### Trie Class
