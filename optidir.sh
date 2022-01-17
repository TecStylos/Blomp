if [[ $# != 1 ]]; then
    echo "Usage: maxvdir.sh [directory]"
    exit
fi

DIRECTORY=$1

for file in $DIRECTORY/{*.png,*.jpg}; do
    if [[ $file != *_DENC* ]]; then
        if [[ $file != *_HEAT* ]]; then
            blomp opti $file -q -g + -m + -i 20
        fi
    fi
done