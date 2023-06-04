#!/bin/bash

if [[ -d /usr/lib/systemd/system ]]; then
  systemctl disable xdbot
  systemctl stop xdbot
  rm -f /usr/lib/systemd/system/xdbot.service
  rm -f /etc/init.d/xdbot
else
  /sbin/chkconfig xdbot off
  /sbin/chkconfig --del xdbot
  /etc/init.d/xdbot stop
  rm -f /etc/init.d/xdbot
fi
rm -rf /usr/local/xdbot
echo "XDBot uninstalled"
