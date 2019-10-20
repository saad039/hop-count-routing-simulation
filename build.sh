echo "Building routing";
cd premake
deb="debug"
rel="release"
if [ "$1" == "$deb" ];
    then
    ./premake5 gmake
    make config=$deb
elif [ "$1" == "$rel" ];
    then
    ./premake5 gmake
    make config=$rel
else 
    echo "Invalid argument"
fi