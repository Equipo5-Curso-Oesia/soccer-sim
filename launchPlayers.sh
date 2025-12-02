#!/bin/bash

CURRENT_DIR=$(pwd)
MAKE_EXEC=$(find . -name Makefile -printf '%d\t%p\n' \
            | sort -n | head -n 1 \
            | sed -r -e 's/^[0-9]+\t//' -e 's/Makefile$//')

# If somo command fails, the next commands will not be executed
cd $MAKE_EXEC && make && cd $CURRENT_DIR
sleep 3;

PLAYER_EXEC=$(find . -name player -type f)

# Field Players
COMAND="\"$PLAYER_EXEC\" \"$1\""
for i in {0..9}
do
  gnome-terminal -- bash -c "$COMAND 555$i false" &
  sleep 1
done

# Goalkeeper Player
gnome-terminal -- bash -c "$COMAND 55560 true" &
#rtdrdrte