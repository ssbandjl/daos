# install deps
sudo ./utils/scripts/install-ubuntu.sh
apt install scons meson python3-pip -y
pip3 install pyelftools

# update golang
wget https://go.dev/dl/go1.23.5.linux-amd64.tar.gz
rm -rf /usr/local/go && tar -C /usr/local -xzf go1.23.5.linux-amd64.tar.gz
export PATH=$PATH:/usr/local/go/bin # add it to bashrc
rm -f /usr/bin/go
go version

or
snap install go --classic

python3
import os
os.environ['PATH']


