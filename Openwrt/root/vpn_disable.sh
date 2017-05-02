#!/bin/sh

ln -sf /etc/config/firewall.raw /etc/config/firewall
/etc/init.d/firewall reload
/etc/init.d/openvpn stop
/etc/init.d/openvpn disable
mosquitto_pub -t "vpn/status" -m "OFF" -q 1 -r
