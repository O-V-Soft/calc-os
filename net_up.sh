sudo ip tuntap add dev tap0 mode tap user $USER
sudo ip addr add 192.168.50.1/24 dev tap0
sudo ip link set tap0 up

sudo sysctl -w net.ipv4.ip_forward=1

sudo iptables -t nat -A POSTROUTING -o wlp2s0 -j MASQUERADE
sudo iptables -A FORWARD -i tap0 -o wlp2s0 -j ACCEPT
sudo iptables -A FORWARD -i wlp2s0 -o tap0 -m state --state RELATED,ESTABLISHED -j ACCEPT