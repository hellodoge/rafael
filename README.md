### The Rafael Programming Language
Rafael is an exprerimental scripting programming language. It has a lisp like syntax and is memory safe by default.

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
