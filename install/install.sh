#!/bin/bash

# user can config the following configs, then package.
#INSTALL=/usr/local/xdbot
INSTALL=/usr/local/xdbot

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

# check lsb_release
ok_msg "check tools"
lsb_release -v >/dev/null 2>&1; ret=$?
if [[ $ret -ne 0 ]]; then failed_msg "no lsb_release, install: yum install -y redhat-lsb"; exit $ret; fi

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

# For systemctl
if [[ -d /usr/lib/systemd/system ]]; then
    ok_msg "install xdbot.service for systemctl"
    (
        cp -f $install_root/usr/lib/systemd/system/xdbot.service /usr/lib/systemd/system/xdbot.service &&
        systemctl daemon-reload
    ) >>$log 2>&1
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install xdbot.service for systemctl failed"; exit $ret; fi
    ok_msg "install xdbot.service for systemctl success"
fi

# install system service
lsb_release --id|grep "CentOS" >/dev/null 2>&1; os_id_centos=$?
lsb_release --id|grep "Ubuntu" >/dev/null 2>&1; os_id_ubuntu=$?
lsb_release --id|grep "Debian" >/dev/null 2>&1; os_id_debian=$?
lsb_release --id|grep "Raspbian" >/dev/null 2>&1; os_id_rasabian=$?
if [[ 0 -eq $os_id_centos ]]; then
    ok_msg "install system service for CentOS"
    if [[ -d /usr/lib/systemd/system ]]; then
        systemctl enable xdbot
    else
        /sbin/chkconfig --add xdbot && /sbin/chkconfig xdbot on
    fi
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install system service failed"; exit $ret; fi
    ok_msg "install system service success"
elif [[ 0 -eq $os_id_ubuntu ]]; then
    ok_msg "install system service for Ubuntu"
    update-rc.d xdbot defaults
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install system service failed"; exit $ret; fi
    ok_msg "install system service success"
elif [[ 0 -eq $os_id_debian ]]; then
    ok_msg "install system service for Debian"
    update-rc.d xdbot defaults
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install system service failed"; exit $ret; fi
    ok_msg "install system service success"
elif [[ 0 -eq $os_id_rasabian ]]; then
    ok_msg "install system service for RaspberryPi"
    update-rc.d xdbot defaults
    ret=$?; if [[ 0 -ne ${ret} ]]; then failed_msg "install system service failed"; exit $ret; fi
    ok_msg "install system service success"
else
    warn_msg "ignore and donot install system service for `lsb_release --id|awk '{print $3}'`."
fi

echo ""
echo "install success, you can start xdbot on CentOS6:"
echo -e "${GREEN}      sudo /etc/init.d/xdbot start${BLACK}"
echo "or CentOS7:"
echo -e "${GREEN}      sudo systemctl start xdbot${BLACK}"
echo "xdbot root is ${INSTALL}"

exit 0
