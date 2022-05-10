if [[ $# != 1 ]]; then
    echo "Usage: optidir.sh [directory]"
    exit
fi

DIRECTORY=$1

for file in $DIRECTORY/{*.png,*.jpg}; do
    if [[ $file != *_DENC* ]]; then
        if [[ $file != *_HEAT* ]]; then
            blomp opti $file -g + -m + -i 0
        fi
    fi
done
