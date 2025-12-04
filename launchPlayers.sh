#!/bin/bash

cd player && make && cd ..
sleep 3;

PLAYER_EXEC=$(find . -name player)

for i in {0..9}
do
  gnome-terminal -- bash -c "\"$PLAYER_EXEC\" Sin_Nombre 555$i false" &
  sleep 1
done

gnome-terminal -- bash -c "\"$PLAYER_EXEC\" Sin_Nombre 5560 true" &