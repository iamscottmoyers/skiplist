# skiplist

This is an implementation of the skiplist datastructure written in C99, although there's
very few uses of the C99 features in it so should be easy to modify for C89.

See https://en.wikipedia.org/wiki/Skip_list for an overview of the skiplist datastructure.

This implementation has a couple of features.
- It's efficiently indexable
- You can specify whether the skiplist should allow duplicate items or work like a set.

Here's the complexity of the operations this data structure provides, where N is the
number of elements in the list:

Operation | Complexity
----------|-----------
Insert    | O(log(N))
Delete    | O(log(N))
Index     | O(log(N))

Each node in a skiplist contains a number of next pointers, the maximum number of pointers
that this skiplist implementation will use for a node is given by SKIPLIST_MAX_LINKS which
is probably set to 32. This number will in theory keep the datastructure working with O(log(N))
performance for around 2^32 data items, although I suspect other factors will destroy the
performance before it gets near that amount of data items.

The number of next pointers adds a constant overhead to the skiplist operations so where
possible it's nice to reduce this, The skiplist_create() function has a size_estimate_log2
parameter for changing this number.

The following graphs show the time per node for lookup and insertion for all numbers of next
nodes and for varying numbers of nodes in the datastructure. These graphs are taken from the
link_trade_off_insert and link_trade_off_lookup tests.

![](https://github.com/iamscottmoyers/skiplist/blob/master/images/link_trade_off_insert.png)
![](https://github.com/iamscottmoyers/skiplist/blob/master/images/link_trade_off_lookup.png)

It's easier to see on a zoomable graph, but most of the time the smaller link numbers are
quicker unless you're using a large number of elements, 32 is still expensive for 100,000
elements so I can't imagine ever needing over 20 links. Over 1,000 elements and the performance
of the small number of next nodes will degrade very quickly to O(N).
