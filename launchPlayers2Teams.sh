#!/bin/bash

# Script to launch two teams for soccer simulation
# Usage: ./launchPlayers2Teams.sh "TeamName1" "TeamName2"

TEAM1="${1:-Team1}"
TEAM2="${2:-Team2}"
BASE_PORT1="${3:-5550}"
BASE_PORT2="${4:-5650}"

CURRENT_DIR=$(pwd)
MAKE_EXEC=$(find . -name Makefile -printf '%d\t%p\n' \
            | sort -n | head -n 1 \
            | sed -r -e 's/^[0-9]+\t//' -e 's/Makefile$//')

# If some command fails, the next commands will not be executed
cd "$MAKE_EXEC" && make && cd "$CURRENT_DIR" || exit 1
sleep 3;

PLAYER_EXEC=$(find . -name player -type f)

if [ -z "$PLAYER_EXEC" ]; then
    echo "Error: player executable not found"
    exit 1
fi

echo "Launching Team 1: $TEAM1 (ports $BASE_PORT1-$((BASE_PORT1+10)))"
echo "Launching Team 2: $TEAM2 (ports $BASE_PORT2-$((BASE_PORT2+10)))"
echo ""

# Function to launch a team
launch_team() {
    local TEAM_NAME="$1"
    local BASE_PORT="$2"
    
    COMAND="\"$PLAYER_EXEC\" \"--team-name=$TEAM_NAME\""
    
    # Field Players (10 players)
    for i in {0..9}
    do
        PORT=$((BASE_PORT + i))
        gnome-terminal -- bash -c "$COMAND --port=$PORT" &
        sleep 1
    done
    
    # Goalkeeper Player
    GK_PORT=$((BASE_PORT + 10))
    gnome-terminal -- bash -c "$COMAND --port=$GK_PORT --is-goalie=true" &
}

# Launch both teams in parallel
launch_team "$TEAM1" "$BASE_PORT1" &
TEAM1_PID=$!

sleep 2

launch_team "$TEAM2" "$BASE_PORT2" &
TEAM2_PID=$!

wait $TEAM1_PID $TEAM2_PID

echo "Both teams launched successfully!"
