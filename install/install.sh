#!/bin/bash

# user can config the following configs, then package.
#INSTALL=/usr/local/xdbot
INSTALL=/usr/local/xdbot

model_directory = /userdata/model

##################################################################################
##################################################################################
##################################################################################
# discover the current work dir, the log and access.
echo "argv[0]=$0"
if [[ ! -f $0 ]]; then 
    echo "directly execute the scripts on shell.";
    work_dir=`pwd`
else 
    echo "execute scripts in file: $0";
    work_dir=`dirname $0`; work_dir=`(cd ${work_dir} && pwd)`
fi
product_dir=$work_dir

log="${work_dir}/logs/package.`date +%s`.log" && . ${product_dir}/scripts/_log.sh && check_log
ret=$?; if [[ $ret -ne 0 ]]; then exit $ret; fi

# user must stop service first.
ok_msg "check previous install"
if [[ -f /etc/init.d/xdbot ]]; then
    /etc/init.d/xdbot status >/dev/null 2>&1
    ret=$?; if [[ 0 -eq ${ret} ]]; then 
        failed_msg "you must stop the service first: sudo /etc/init.d/xdbot stop"; 
        exit 1; 
    fi
fi
ok_msg "previous install checked"

# backup old xdbot
ok_msg "backup old xdbot"
install_root=$INSTALL
#install_bin=$install_root/bin/macchina
if [[ -d $install_root ]]; then
    #version="unknown"
    #if [[ -f $install_bin ]]; then
    #    version=`$install_bin -h 2>/dev/stdout 1>/dev/null`
    #fi
    
    #backup_dir=${install_root}.`date "+%Y-%m-%d_%H-%M-%S"`.v-$version
    backup_dir=${install_root}.`date "+%Y-%m-%d_%H-%M-%S"`
    #ok_msg "backup installed dir, version=$version"
    ok_msg "backup installed dir, version=1.0.0"
    ok_msg "    to=$backup_dir"
    mv $install_root $backup_dir >>$log 2>&1
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "backup installed dir failed"; exit $ret; fi
    ok_msg "backup installed dir success"
fi
ok_msg "old xdbot backuped"

# prepare files.
ok_msg "prepare files"
(
    sed -i "s|^ROOT=.*|ROOT=\"${INSTALL}\"|g" ./etc/init.d/xdbot
) >> $log 2>&1
ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "prepare files failed"; exit $ret; fi
ok_msg "prepare files success"

# copy core files
ok_msg "copy core components"
(
    mkdir -p $install_root
    mkdir -p $model_directory
    cp -r $work_dir/lib $install_root &&
    cp -r $work_dir/etc $install_root &&
    cp -r $work_dir/usr $install_root &&
    cp -r $work_dir/bin $install_root  
) >>$log 2>&1
ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "copy core components failed"; exit $ret; fi
ok_msg "copy core components success"

# install init.d scripts
ok_msg "install init.d scripts"
(
    rm -rf /etc/init.d/xdbot &&
    ln -sf $install_root/etc/init.d/xdbot /etc/init.d/xdbot
) >>$log 2>&1
ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install init.d scripts failed"; exit $ret; fi
ok_msg "install init.d scripts success"

echo ""
echo "install success, you can start xdbot:"
echo -e "${GREEN}      sudo /etc/init.d/xdbot start${BLACK}"
echo "xdbot root is ${INSTALL}"

exit 0
