#!/bin/bash


#### 生成 setup升级包， 苏标平台升级包


#平台要求命名规则：
#  终端通过JT/T 808中的终端控制指令对终端进行升级，升级文件命名规则如下：

#  <设备类型>_<厂家编号>_<设备型号>_<依赖软件版本号>_<软件版本号>.<后缀名>。

#  字段定义如下：

#  设备类型：01——终端；02——保留；03——ADAS；04——DSM；05——BSD；06——TPMS。

#  厂家ç号：设备厂家名称编号，由数字和字母组成。

#  设备型号：由设备厂家定义的设备型号，由数字和字母组成。

#  依赖软件版本号：软件升级需要依赖的软件版本，由数字和字母组成。

#  软件版本号：本次升级的软件版本，由数字和字母组成。

#  后缀名：设备厂家自定义升级文件后缀名，由数字和字母组成



install="install"
pack_name="subiao_netprot"
setup_tgz=$pack_name".tgz"

prot_tgz=$pack_name".tgz"
prot_algo_tgz=$pack_name".tgz"

setup_dir="setup"
prot_dir="prot"
prot_algo_dir="prot_algo_dir"
prot_algo="prot_algo"
algo="algo"

upgrade_bin="03_MINIEYE_M4_1.1.1_1.1.5.bin"

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

# create install path
mkdir -p $install
git_folder=`git_reversion`
src_dir="output/$git_folder/strip_target/"
dst_dir=$pack_name

# cp release here
cd $install;rm -r $dst_dir;cd ..
cp -r $src_dir $install"/"$dst_dir
cd $install;

if [ $# != 1 ]
then
    echo "input arg: setup/prot/prot_algo"
    exit 1;
fi

if [ "$1" == "prot" ] #########pack for prot
then
    echo -e "pack for prot...\n"
    mkdir -p $prot_dir
    cd $dst_dir
    if [ $? == 0 ]
    then
        tar czf $prot_tgz *
        cp $prot_tgz "../"$prot_dir"/"$upgrade_bin
        mv $prot_tgz "../"$prot_dir
        cd "../"$prot_dir
        md5sum *
    fi
elif [ "$1" == "prot_algo" ] ############pack for algo
then
    echo -e "pack for prot algo...\n"
    mkdir -p $prot_algo
    mkdir -p $prot_algo_dir
    cp $dst_dir"/"* $prot_algo_dir

    if [ ! -d algo ]
    then
        cp -r "../"$algo .
        cp $algo"/"* $prot_algo_dir
    fi

    cd $prot_algo_dir
    if [ $? == 0 ]
    then
        tar czf $prot_algo_tgz *
        cp $prot_algo_tgz "../"$prot_algo"/"$upgrade_bin
        mv $prot_algo_tgz "../"$prot_algo
        cd "../"$prot_algo
        md5sum *
    fi
elif [ "$1" == "setup" ] ####################pack for setup
then
    echo -e "pack for setup...\n"
    mkdir -p $setup_dir

    file="AdasProt  adasProt.sh  DsmProt  dsmProt.sh"
    file2="prot.json  prot_upgrade.sh"
    tar czf $setup_tgz $dst_dir

    # md5 start
    cd $dst_dir
    if [ $? == 0 ]
    then
        echo "file list:"; ls; echo "do mdsum:"
        md5sum * >../md5.txt
        cat ../md5.txt
    fi
    cd ..

    mv $setup_tgz $setup_dir
    cp md5.txt $setup_dir"/"install.txt
    rm md5.txt
    cd $setup_dir

    for s in $file
    do
        sed -i 's/'$s'/\/system\/bin\/'$s'/' install.txt
    done

    for s in $file2
    do
        sed -i 's/'$s'/\/data\/'$s'/' install.txt
    done
    echo -e "pack setup done\n"
    # md5 end
    ####################################### pack setup end

    echo "$setup_tgz packed in $install"

else
    echo "input arg: setup/prot/prot_algo"
fi

