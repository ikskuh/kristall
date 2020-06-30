FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y git wget fuse qttools5-dev-tools
RUN apt-get install -y qt5-default qt5-qmake qtmultimedia5-dev libqt5svg5-dev libssl-dev make g++

RUN wget -O /usr/local/bin/linuxdeploy https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-$(uname -m).AppImage
RUN chmod +x /usr/local/bin/linuxdeploy
RUN file $(which linuxdeploy)

RUN wget -O /usr/local/bin/linuxdeployqt https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-$(uname -m).AppImage
RUN chmod +x /usr/local/bin/linuxdeployqt
RUN file $(which linuxdeployqt)

VOLUME /artifacts

ENTRYPOINT /compile.sh

COPY compile.sh /compile.sh
