
Blossom
=======

This is an implementation of the Blossom algorithm in O(nm a(n)) where n is the
number of vertices in the graph, m the number of edges and a is the inverse
Ackermann function.

Input
-----

It expects a description of the graph on its standard input. The graph is a
succession of number. First is the number of nodes n, then the number of edges m,
then the edges as couples of numbers (identifier of nodes between 0 included and
n not included). For example, the cycle of size 5 would be :

```
5 5
0 1
1 2
2 3
3 4
4 0
```

Output
------

The program outputs the matching in the form of a list of edges, each being two
nodes identifiers separated by `--`. For the cycle of size 5, an output could
be :

```
0 -- 1
2 -- 3
```
