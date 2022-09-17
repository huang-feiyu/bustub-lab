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

Normal stuff. It's weird that I cannot use `std::make_unique` sometimes.

### DEBUG

<b>*</b> insert last failed => bug01

```c
- (*prev)->InsertChildNode(*key.end().base(),
-                          std::make_unique<TrieNodeWithValue<T>>(*key.end().base(), value));
+ (*prev)->InsertChildNode(*(key.end() - 1).base(),
+                          std::make_unique<TrieNodeWithValue<T>>(*(key.end() - 1).base(), value));
```
