## The Rafael Programming Language
Rafael is an exprerimental scripting programming language.\
It has a lisp like syntax and is memory safe by default.

### Example
The following program outputs [Pascal's Triangle](https://en.wikipedia.org/wiki/Pascal%27s_triangle)
```
(import range)

(def print_triangle height
    (for x (range &height)
        (print (repeat (+ &height (- &x)) ' '))
        (set numbers 1)
        (for y (range 1 (+ &x 1))
            (set last (index -1 &numbers))
            (set numbers &numbers
                (/ (* &last (+ &x (- &y) 1)) &y)
            )
        )
        (print (join ' ' &numbers) '\n')
    )
)

(print "Input height of triangle: ")
(print_triangle (int (input)))
```

### Usage
```
$ rafael triangle.rf
Input height of triangle: 5

         1
       1   1
     1   2   1
   1   3   3   1
 1   4   6   4   1
```

### REPL
```
$ rafael
--> (set is_negative (lambda x (< &x 0)))
(lambda)
--> (set a
...     (for x (range 1 11)
...         (if (== (type (/ &x 2)) int)
...             (get x)
...         else
...             (- &x)
... )))
(-1 2 -3 4 -5 6 -7 8 -9 10)
--> (filter)
Error! No macro specified by filter
--> (import filter)
--> (filter &is_negative &a)
(-1 -3 -5 -7 -9)
```

### Installation (Linux)
```
rm -rf /tmp/rafael_repo
git clone https://github.com/hellodoge/rafael.git /tmp/rafael_repo
rm -rf /tmp/rafael_build
mkdir /tmp/rafael_build
cd /tmp/rafael_build
cmake /tmp/rafael_repo/
make
test -d $HOME/bin || mkdir $HOME/bin
mv /tmp/rafael_build/rafael $HOME/bin/
test -d $HOME/.local/lib/rafael || mkdir $HOME/.local/lib/rafael
mv /tmp/rafael_repo/stdlib/* $HOME/.local/lib/rafael/
```

### Running tests
```
rm -rf /tmp/rafael_repo
git clone https://github.com/hellodoge/rafael.git /tmp/rafael_repo
rafael -t /tmp/rafael_repo/tests/*.rf /tmp/rafael_repo/tests/stdlib/*.rf
```

### File format
Rafael programs are UTF-8 files with the .rf file extension.
