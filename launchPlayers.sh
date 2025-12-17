#!/bin/bash

CURRENT_DIR=$(pwd)
MAKE_EXEC=$(find . -name Makefile -printf '%d\t%p\n' \
            | sort -n | head -n 1 \
            | sed -r -e 's/^[0-9]+\t//' -e 's/Makefile$//')

# If somo command fails, the next commands will not be executed
cd $MAKE_EXEC && make && cd $CURRENT_DIR
sleep 1;

PLAYER_EXEC=$(find . -name player -type f)

# Field Players
COMAND="\"$PLAYER_EXEC\" \"--team-name=$1\""
for i in {0..9}
do
  gnome-terminal -- bash -c "$COMAND --port=$2$i > ${MAKE_EXEC}player$1$i.txt" &
  sleep 1
done

# Goalkeeper Player
gnome-terminal -- bash -c "$COMAND --port=$(($2 + 1))0 --is-goalie=true > ${MAKE_EXEC}player${1}10.txt" &
