#!/system/bin/sh

upgrade_dir="/mnt/obb/prot_upgrade"
cd $upgrade_dir

target_bin="subiao_upgrade.bin"
target_tgz="subiao_upgrade.tar.gz"

busybox mv $target_bin $target_tgz
echo "Extracting target.tgz..."
busybox tar xzf $target_tgz

mount -o remount,rw /system

upgrade_bin="AdasProt DsmProt adasProt.sh dsmProt.sh"
bin_dir="/system/bin/"

for bin in $upgrade_bin
do
    echo "list $bin"

    if [ -f $bin ]; then
        echo "update $bin..."
        busybox chmod 0755 $bin
        busybox mv $bin $bin_dir$bin".new"
        busybox mv $bin_dir$bin".new" $bin_dir$bin
        sync
    else
        echo "$bin not exsist"
    fi
done

upgrade_conf="prot.json prot_upgrade.sh"
conf_dir="/data/"

for conf in $upgrade_conf
do
    echo "list $conf"

    if [ -f $conf ]; then
        echo "update $conf..."
        busybox chmod 0755 $conf
        busybox mv $conf $conf_dir$conf".new"
        busybox mv $conf_dir$conf".new" $conf_dir$conf
        sync
    else
        echo "$conf not exsist"
    fi
done

sync
echo -e "update prot done !\n"


###    update algo
algo_dir="./"

algo_update="install_mpk.sh"

for mpk in `find $algo_dir -name "*.mpk"`
do
    echo "updating $mpk ..."
    $algo_update $mpk
    echo "updating $mpk done"
    rm $mpk
    echo -e "delete $mpk\n"
done

sync

rm $target_tgz
echo "update algo done !"


