if [[ $# != 1 ]]; then
    echo "Usage: maxvdir.sh [directory]"
    exit
fi

DIRECTORY=$1

for file in $DIRECTORY/{*_DENC*,*_HEAT*,*_OPTI*,*_MAXV*,*.blp}; do
    rm $file
done