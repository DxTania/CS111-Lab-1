## Student: Carlos Sotelo UID: 303891983

## Student: Tania DePasquale UID: 704018998

It is unclear to us whether or not our parser is supposed to handle
sequence commands with a single simple command, such as "a;". If so, it is not
clear how to setup a command struct with it.

===============================================================================

# Notes:

Sequence commands when printed will print out (null) if there is no second command.
This has me believe that we should parse something such as

```
echo hello;

echo world
```

As one sequence command, even with the newline character. This is how we do this now.

Charly: This has been handled.
===============================================================================

# Tania's Notes for Lab 1C:

I created a vector class for representing the listof nodes in the graph, each node
contains a vector of files (strings) associated with it, and a vector of
node pointers to the nodes the current one must wait for.

I've implemented the dependency lists, although probably not in the most efficient way.
Using the function print() will print out each command and its commands that need
to be run first. I am unsure whether we should keep a list of grandparents here
or if just immediate parents (same exact file) should be here.

Anyways, did that like this:

`get_files` takes a command and returns a vector of strings that represent the
"files" it acts on (I say "files" because we are supposed to just include
options here).

`get_has_dependences` takes a node and both updates its before vector with
notes that have file intersections, and returns true if hte node did have
dependencies. 

`add_command` adds the command to our dependency graph. (or no depency graph)

`speed_of_light` attempts parallelization (still didn't get this working)

`print` prints each node in deps/nodeps along with its dependencies if it
has any.

===============================================================================

#Side-note:

Are we supposed to ignore any order we may have wanted from a script?
Like say we did:

```
echo hello

echo world
```

2 different commands, neither depends on the other. If we parallize this
it might print out world hello instead of hello world, right? Is that ok?
