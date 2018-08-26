#_
$ echo 1
- 1
$ su
- Password:
# echo HELLO
- HELLO
# exit
$ echo 42\
> \<foo\>
- 42<foo>
$ echo $'\x1b[31mred \x1b[91mbright \x1b[1mbold'
- [31mred [91mbright [1mbold
$ echo $'\x1b[38;5;44m38;5;44m'
- [38;5;44m38;5;44m
$ echo $'\x1b[38;5;24;48;5;153;1m' QWX
- [38;5;24;48;5;153;1m QWX
#_

