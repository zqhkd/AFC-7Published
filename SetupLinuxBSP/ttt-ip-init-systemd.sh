#!/bin/sh

REF_ETH_INTERFACE=end1
IP_REF_NAME=42080000.bus/42080000.bus:ttt-sw@4c000000/4c000000.deip-sw

# read mac address
get_mac() {
    read MAC </sys/class/net/$REF_ETH_INTERFACE/address
    echo "[INFO]: Mac Address of $REF_ETH_INTERFACE: $MAC"
}

get_soc_path() {
    devicetree_path=$(ls -1 -d /sys/devices/platform/* | grep "/soc" | head -n 1)
    if [ -d "$devicetree_path" ];
    then
        SOC_PATH=$devicetree_path
    else
        echo "[ERROR]: /sys/devices/platform/soc* is not available"
        echo ""
        exit 1
    fi

}

wait_sysfs() {
    path=$1
            for i in $(seq 0 5)
        do
            if [ ! -e "$path" ]; then
                break;
            else
                sleep 0.5s
            fi
        done
}

st_configure() {
    echo "[INFO]: Skip TTT Switch configuration for AFC-7 (Single ETH1 Mode)."
    # 把原来对 $IP_REF_NAME 的检测和 echo 全部注释掉
}

# Set the interfaces up like in the interfaces files
# Usage: set_interfaces_up
set_interfaces_up()
{
    get_mac
    # 如果你需要给原生网口 end1（即原eth1）配置静态IP，直接操作 end1
    ip link set dev end1 address $MAC
    ip link set dev end1 up
    ip addr add 192.168.0.10/24 dev end1
}

# Set the interfaces down like in the interfaces files
# Usage: set_interfaces_down
set_interfaces_down()
{
    ip link set dev br0 down
    ip link delete dev br0

    ip link set dev sw0ep down
}

# Start the deamons as they would do at start
# Usage: start_daemons
start_daemons()
{
    # stop NTP service
    systemctl stop systemd-timesyncd
    systemctl stop ntpd

    ip link set br0 type bridge stp_state 1

    mstpctl addbridge br0
    mstpctl setforcevers br0 mstp
    mstpctl setvid2fid br0 0:1

    systemctl start lldpd &

    systemctl start deptp &

    #systemctl start snmpd &

    /usr/share/netopeer2-server/netopeer2-server-service start &
}

# Stop the daemons
# Usage: stop_daemons
stop_daemons()
{
    mstpctl delbridge br0
    #systemctl stop snmpd &
    systemctl stop lldpd &
    systemctl stop deptp &
    /usr/share/netopeer2-server/netopeer2-server-service stop
}

start()
{
    echo "[INFO]: ST configuration of IP"
    st_configure
    echo "[INFO]: ST set brigde interface"
    set_interfaces_up
    echo "[INFO]: start service"
    start_daemons
}

stop() {
    stop_daemons
    set_interfaces_down
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    restart)
        stop
        start
        ;;
    restore)
        /usr/share/netopeer2-server/netopeer2-server-service restore
        ;;
esac
exit 0
