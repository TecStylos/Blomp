if [[ $# != 1 ]]; then
    echo "Usage: maxvdir.sh [directory]"
    exit
fi

DIRECTORY=$1

for file in $DIRECTORY/{*.png,*.jpg}; do
    if [[ $file != *_DENC* ]]; then
        if [[ $file != *_HEAT* ]]; then
            blomp opti $file -q -b -o ${file%.*}_DENC.png -m ${file%.*}_HEAT.png -i 20
        fi
    fi
done
