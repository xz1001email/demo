#!/bin/bash


function git_reversion()
{
    git_version=`git rev-parse --verify --short HEAD`
    if [ -z "$(git status --untracked-files=no --porcelain)" ]; then
        git_version+=""
    else
        git_version+="_M"
    fi
    echo -n "${git_version}"
}

dms_target="DsmProt"
adas_target="AdasProt"
output_folder="output/"

device=$1
interface=$2
target=wsi
buildpath="arm/"
buildtarget=$buildpath"bin/"$target

mkdir -p $buildpath
cd $buildpath

if [ "$device" == "adas" -a "$interface" == "tcp" ]
then
    target_dist=$adas_target
    option="-DADAS=true -DDMS=false -DTCP=true -DRS232=false"
    echo "./building for adas tcp"
    ../android_cmake.sh $option

elif [ "$device" == "adas" -a "$interface" == "rs232" ]
then
    target_dist=$adas_target
    option="-DADAS=true -DDMS=false -DTCP=false -DRS232=true"
    echo "./building for adas rs232"
    ../android_cmake.sh $option

elif [ "$device" == "dms" -a "$interface" == "tcp" ]
then
    target_dist=$dms_target
    option="-DADAS=false -DDMS=true -DTCP=true -DRS232=false"
    echo "./building for dms tcp"
    ../android_cmake.sh $option

elif [ "$device" == "dms" -a "$interface" == "rs232" ]
then
    target_dist=$dms_target
    option="-DADAS=false -DDMS=true -DTCP=false -DRS232=true"
    echo "./building for dms rs232"
    ../android_cmake.sh $option
else
    echo "usage: [option] adas/dms, tcp/rs232"
    exit 1
fi

make
cd ..

outputpath=$output_folder`git_reversion`"/"
mkdir -p $outputpath
echo "generate target $outputpath"

echo "cp $buildtarget $outputpath$target_dist"
cp $buildtarget $outputpath$target_dist

strip=/home/xiao/bin/ndk/android-ndk-r11c/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip
stripdir="strip_target/"
echo "mkdir $outputpath$stripdir"
mkdir -p $outputpath$stripdir
cp $buildtarget $outputpath$stripdir$target_dist
echo "strip..."
$strip $outputpath$stripdir$target_dist

cp adasProt.sh dsmProt.sh prot_upgrade.sh prot.json $outputpath$stripdir



