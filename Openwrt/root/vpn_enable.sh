#!/bin/sh

ln -sf /etc/config/firewall.vpn /etc/config/firewall
/etc/init.d/firewall reload
/etc/init.d/openvpn enable
/etc/init.d/openvpn start
mosquitto_pub -t "vpn/status" -m "ON" -q 2 -r
