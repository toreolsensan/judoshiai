#!/bin/sh
DIR=$HOME/bin
echo -n "Installation directory [$DIR]: "
read answer
if [ "X$answer" != "X" ] ; then
    DIR=$answer
fi
echo "Installation directory $DIR"

if [ ! -d $DIR ] ; then
    echo -n "Directory does not exist, create? [Y/n] "
    read answer
    case $answer in
	[Nn] ) exit 0 ;;
    esac
    mkdir -p $DIR
fi

cp -r judoshiai $DIR

if [ -d $HOME/Desktop ] ; then 
    echo -n "Create a desktop entry? [Y/n] "
    read answer
    case $answer in
	[Nn] ) exit 0
    esac

    cat <<EOF >$HOME/Desktop/JudoShiai.desktop
[Desktop Entry]
Name=JudoShiai
Exec=$DIR/judoshiai/bin/judoshiai
Terminal=false
Type=Application
Icon=$DIR/judoshiai/etc/judoshiai.png
Categories=Application
EOF
    
    cat <<EOF1 >$HOME/Desktop/JudoTimer.desktop
[Desktop Entry]
Name=JudoTimer
Exec=$DIR/judoshiai/bin/judotimer
Terminal=false
Type=Application
Icon=$DIR/judoshiai/etc/judotimer.png
Categories=Application
EOF1

    cat <<EOF2 >$HOME/Desktop/JudoInfo.desktop
[Desktop Entry]
Name=JudoInfo
Exec=$DIR/judoshiai/bin/judoinfo
Terminal=false
Type=Application
Icon=$DIR/judoshiai/etc/judoinfo.png
Categories=Application
EOF2

fi
