# Extendible Hash Index

> 2021 fall [PROJECT #2 - EXTENDIBLE HASH INDEX](https://15445.courses.cs.cmu.edu/fall2021/project2/)

[TOC]

## Prepare

The disk-backed hash table is responsible for fast data retrieval without
having to search through every record in a database table.

In this project, we are going to implement extendible hash table for indexing.

There is two main types of page we need to handle:
* Directory Page: Record page id for each bucket; To locate a bucket, use the
  LSBs of hash value on a key. Then, get the bucket's page id via one-to-one
  array.
* Bucket Page: Store K-V{key, record id} pairs; Search linearly by hash value

The rest job is to follow extendible hash table scheme to implement
`Insert`, `Remove`, `GetValue`. And we also need to support multi-threading,
aka. Concurrency Control. (Much easier than B<sup>+</sup>Tree)

## Task#1: Page Layout

`bucket_id` is the hash value of a specific key.

* Directory Page
  * `GetSplitImageIndex`: Get the "split image"(brother bucket) bucket id =>
    Just NOT the highest bit of its bucket
  * `GetLocalHighBit`: Only for function above
  * `IncrGlobalDepth`: Because we use LSB to index bucket, so when we double
    the directory(inc global depth), we need to map our new bucket to its
    brother. Specifically, just need to copy bucket page id and local depth.
* Bucket Page
  * `readable_` & `occupied_`: We do not want to remove slot immediately. So
    when remove one slot, do not remove it but clear `readable_` flag.<br/>
    Each bit of the 2 members tells us whether the slot(with specific bucket id)
    is readable.
  * Do **not** place a member right after the `array_`, it will change the elem
    of the `array_`

There are some helper functions that I do not understand.

## Task#2: Hash Table Implementation

> For concurrency control now, I just use a big Writer Latch to make everything
> consequential.

Take a look at [Extendible Hashing](https://github.com/nitish6174/extendible-hashing).

* bucket splitting: Split a bucket if there is no room for insertion,
  i.e. insert to a full bucket.
* bucket merging: Merging must be attempted when a bucket becomes empty.
  * Only empty buckets can be merged.
  * Buckets can only be merged with their split image if their split image has
    the same local depth.
  * Buckets can only be merged if their local depth is greater than 0.
* directory growing: When bucket is full and cannot split it, then grow the
  directory.
* directory shrinking: Only shrink the directory if the local depth of every
  bucket is strictly less than the global depth of the directory.

Insert: Check whether the bucket is full: if not, just insert it.
Otherwise, follow the steps below:
1. Before splitting, increment local depth
2. Check if hash table has to **grow directory**: if is(LD > GD), increment GD 
3. Initialize a split image bucket page
4. Re-hash the existing k/v pairs into split image bucket and previous bucket
5. Re-organize previous buckets, the prev half points to bkt, the next half
   points to img
6. Insert the k/v until success or fail

Remove: Remove k/v, check whether the bucket is empty: if is, return.
Otherwise, follow the steps below:
1. Check three primitives
2. Delete bucket page
3. Prev bkt points to img page
4. Decrement local depth
5. Re-organize all buckets
6. Shrink if needed

### Debug

* Infinite test => bug00

```diff
  if (!bkt_page->IsEmpty() ||                                                // premise 1
      dir_page->GetLocalDepth(bkt_id) == 0 ||                                // premise 2
      dir_page->GetLocalDepth(bkt_id) != dir_page->GetLocalDepth(img_id)) {  // premise 3
+   table_latch_.WUnlock();
    return;
  }
```

* Delete no-unique key failed => bug01

Sample test wrong...

```diff
  // delete some values
  for (int i = 0; i < 5; i++) {
    EXPECT_TRUE(ht.Remove(nullptr, i, i));
-   std::vector<int> res;
+   res.clear();
    ht.GetValue(nullptr, i, &res);
    if (i == 0) {
      // (0, 0) is the only pair with key 0
      EXPECT_EQ(0, res.size());
    } else {
      EXPECT_EQ(1, res.size());
      EXPECT_EQ(2 * i, res[0]);
    }
  }
```

* SplitInsert segmentation fault => bug02

```diff
- for (uint32_t i = bkt_id; i >= diff; i -= diff) {
+ for (uint32_t i = bkt_id - diff; i >= diff; i -= diff) {
    dir_page->SetBucketPageId(i, bkt_page_id);
    dir_page->SetLocalDepth(i, local_depth);
  }
 ``` 

* SplitInsert assertion failed => bug03

```diff
  dir_page->SetBucketPageId(img_id, img_page_id);  // add to dir_page
+ dir_page->SetLocalDepth(img_id, dir_page->GetLocalDepth(bkt_id));
  assert(dir_page->GetLocalDepth(bkt_id) == dir_page->GetLocalDepth(img_id));
```

* Big latch concurrency issue => bug04

```diff
- assert(bkt_page->IsFull());  // NOTE: Might wrong, concurrency issue
+ if (!bkt_page->IsFull()) {
+   assert(buffer_pool_manager_->UnpinPage(directory_page_id_, false));
+   assert(buffer_pool_manager_->UnpinPage(bkt_page_id, false));
+   table_latch_.WUnlock();
+   return Insert(transaction, key, value);
+ }
```

* GradeScope GrowShrinkTest GD assertion failed => bug05

[Test files](https://github.com/smilingpoplar/cmu-15445/tree/main/test)

It's weird: local GrowShrinkTest is OK, but failed in GradeScope.
