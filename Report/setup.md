# 基本的な環境設定
## VMware Toolsのインストール
#`
$ cd ~/Downloads
$ cp /run/media/rsk0315/VMware\ Tools/VMwareTools-*.tar.gz ./
$ tar xvf VMwareTools-*.tar.gz
$ cd vmware-tools-distrib/
$ sudo ./vmware-install.pl
#`

## `sudo`の設定
以下のコマンドを用いて`/etc/sudoers/`を編集し，いちいちパスワードを求められないようにする．
#`
$ sudo visudo
#`
差分は次の通り．
#`
$ sudo diff -up ~/tmp/sudoers /etc/sudoers
--- /home/rsk0315/tmp/sudoers
+++ /etc/sudoers
@@ -99,7 +99,7 @@ root	ALL=(ALL) 	ALL
 %wheel	ALL=(ALL)	ALL
 
 ## Same thing without a password
-# %wheel	ALL=(ALL)	NOPASSWD: ALL
+%wheel	ALL=(ALL)	NOPASSWD: ALL
 
 ## Allows members of the users group to mount and unmount the 
 ## cdrom as root
#`

## `git`のアップグレード
ビルドするのに必要なライブラリ群をインストールする．
#`
$ sudo yum install openssl-devel curl-devel expat-devel
#`
`gettext-devel`，`perl-devel`，`zlib-devel`，`perl-ExtUtils-MakeMaker`も依存している？ 最初から入っているかも？

古い`git`が入っていることを期待し，以下を実行．
#`
$ git clone https://github.com/git/git.git
$ cd git
$ make && make install
#`
`prefix`は`$(HOME)`になっているので，`$PATH`を見てちゃんと新しいのが実行されるかを確認する．

## `vim`のアップグレード
#`
$ sudo yum install libX11-devel libXt-devel gtk2-devel ncurses-devel
#`
`atk-devel`も依存している？
#`
$ git clone https://github.com/vim/vim.git
$ cd vim
$ ./configure --prefix=$HOME --build=x86_64-redhat-linux --with-x \
>     CFLAGS=-I/usr/include/X11
$ make && make install
#`

## `emacs`のインストール
`http://ftp.jaist.ac.jp/pub/GNU/emacs/`などから最新のものをダウンロード．
#`
$ sudo yum install gnutls-devel
$ cd ~/Downloads
$ curl http://ftp.jaist.ac.jp/pub/GNU/emacs/emacs-26.1.tar.xz -o emacs-26.1.tar.xz
$ tar xvf emacs-26.1.tar.xz
$ cd emacs-26.1/
$ ./configure --prefix=$HOME --build=x86_64-redhat-linux --without-x 
$ make && make install
#`

## `python3`のインストール
#`
$ sudo yum install https://centos7.iuscommunity.org/ius-release.rpm
$ sudo yum install python36u{,-{libs,devel,pip}}
#`

## `bash`のアップグレード
#`
$ curl http://ftp.gnu.org/gnu/bash/bash-5.0-alpha.tar.gz -o bash-5.0-alpha.tar.gz
$ tar xvf bash-5.0-alpha.tar.gz 
$ cd bash-5.0-alpha/
$ ./configure --prefix=$HOME --build=x86_64-redhat-linux
$ make && make install
$ cp doc/bash.1 ~/usr/share/man/man1/
#`

## その他有用なものたちのインストール
#`
$ sudo yum install php
#`

# 発展的な環境構築
趣味の領域に含まれると思われるもの．

## `bashrc`の編集
最初の状態では以下の内容しかなく，そっけなさすぎる（インストールの手順にもよるとは思うが）．
#`
# .bashrc

# Source global definitions
if [ -f /etc/bashrc ]; then
	. /etc/bashrc
fi

# Uncomment the following line if you don't like systemctl's auto-paging feature:
# export SYSTEMD_PAGER=

# User specific aliases and functions
#`

### エイリアス
ファイルの移動時に上書き確認をするのは基本中の基本．
#`
alias rm='rm -iv' cp='cp -iv' mv='mv -iv'
#`

### シェルオプション
リダイレクトの上書きを防ぐのも基本．
#`
set -o noclobber  # set -C でも同じ
#`

### ページャのオプション
#`
export LESS=Fr
#`
- `less -F`：一画面で収まるならそのまま表示
- `less -r`：エスケープシーケンスを解釈


