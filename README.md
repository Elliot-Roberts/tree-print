This is a helper tool for printing binary trees in an easy to inspect format.  
Example of output:
```
 ╭────────────────[5]─╮
 │                    │
[0]───────╮          [5]──────────────╮
          │                           │
    ╭────[2]─╮                     ╭─[12]─╮
    │        │                     │      │
   [0]─╮    [2]─╮           ╭────[10]    [13]─╮
       │        │           │                 │
      [1]      [4]       ╭─[7]─╮             [14]
                         │     │
                        [6]   [7]
```

To add to your project:
```
#include "treeHelper.h"
```
and compile with `--std=c++17` or newer.

To use in code, you make a `WrappedTree` like so:
```c++
Node * my_tree = /* ... */;
WrappedTree<Node> wrapped(my_tree);
```
where `Node` is your binary tree node type.  
The node is expected to have an accessable member named `data` that is able to be printed like so:
```c++
Node * my_node = /* ... */;
cout << my_node->data;
```
To pretty-print your tree, `cout` the `WrappedTree`:
```c++
WrappedTree<Node> wrapped(my_tree);
cout << wrapped;
```

There is also a work-in-progress "comparison" function for seeing how two trees differ:
```c++
Node * my_tree = /* ... */;
WrappedTree<Node> before(my_tree);
remove_even_nodes(&my_tree);  // example operation modifying tree
WrappedTree<Node> after(my_tree);
auto [old_tree, new_tree] = before.compare_to(after);  // returns pair of `WrappedTree`s
cout << old_tree;  // shows the tree before the operation, with removed parts highlighted with color
cout << new_tree;  // shows the tree after the operation, with the updated and new parts highlighted with color
```
