#!/bin/sh

depcheck() {
    [ -z `which $1` ] && echo "$1 needs to be installed to use codim." && exit 1
}
depcheck luajit
depcheck ffmpeg
depcheck espeak

INSTALLDIR='/usr/local/share/lua/5.1/codim'
mkdir -p $INSTALLDIR
cp -r codim/*.lua $INSTALLDIR
chmod 755 $INSTALLDIR
install -m755 codim.lua /usr/local/bin/codim