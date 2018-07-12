# turn this pc into a dumb router
sudo sysctl -w net.ipv4.ip_forward=1

#
# sudo iptables -A FORWARD -i enp1s0 -o wlp2s0 -j ACCEPT
# sudo iptables -A FORWARD -i wlp2s0 -o enp1s0 -m state --state ESTABLISHED,RELATED -j ACCEPT
# sudo iptables -t nat -A POSTROUTING -o wlp2s0 -j MASQUERADE

sudo iptables -t mangle -N DIVERT
sudo iptables -t mangle -A PREROUTING -p tcp -m socket -j DIVERT
sudo iptables -t mangle -A DIVERT -j MARK --set-mark 1
sudo iptables -t mangle -A DIVERT -j ACCEPT

sudo ip rule add fwmark 1 lookup 100
sudo ip route add local 0.0.0.0/0 dev lo table 100


# sudo iptables -t mangle -A PREROUTING ! -d 192.168.1.100 -p tcp -j TPROXY --on-port 82 --on-ip 0.0.0.0 --tproxy-mark 0x1/0x1
sudo iptables -t mangle -A PREROUTING -p tcp --dport 80 -j TPROXY --tproxy-mark 0x1/0x1 --on-port 8080
