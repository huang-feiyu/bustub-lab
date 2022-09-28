# C++ PRIMER

> [fall2022: project 0](https://15445.courses.cs.cmu.edu/fall2022/project0/)

## TEMPLATED TRIE

Before starting, read the debug info in *src/include/common/logger.h*.

* ERROR > WARN > INFO > DEBUG > TRACE (Refers to
  [*When to use different log levels*](https://stackoverflow.com/questions/2031163/when-to-use-the-different-log-levels))

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

<b>*</b> insert last letter failed => bug01

```diff
- (*prev)->InsertChildNode(*key.end().base(),
-                          std::make_unique<TrieNodeWithValue<T>>(*key.end().base(), value));
+ (*prev)->InsertChildNode(*(key.end() - 1).base(),
+                          std::make_unique<TrieNodeWithValue<T>>(*(key.end() - 1).base(), value));
```

<b>*</b> insert lost => bug02

When I remove node from children, I also remove node's children.

```output
== Init: Trie
[.]

== Insert: (aaa,5)
[.]
  [a]
    [a]
      [a](5)

== Insert: (aa,6)
[.]
  [a]
    [a](6)

== Insert: (a,7)
[.]
  [a](7)
```

Weird C++ grammer:

```c++
// case 2: non-terminal node
auto temp_node = std::move(*ptr);
auto tml_node = std::make_unique<TrieNodeWithValue<T>>(std::move(*temp_node), value);
(*prev)->RemoveChildNode(*(key.end() - 1).base());
(*prev)->InsertChildNode(*(key.end() - 1).base(), std::move(tml_node));
```


## CONCURRENT TRIE

* `GetValue`: `RwLatch->RLock()`
* `Insert`: `RWLatch->WLock()`
* `Remove`: `RwLatch->WLock()`

Easy to implement.

---

```
Time:   9 Hours
Score:  100/100
Author: Huang
Date:   2022-09-17
```

