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
