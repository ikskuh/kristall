FROM ubuntu:18.04

RUN apt-get update
RUN apt-get install -y git
RUN apt-get install -y qt5-default qt5-qmake qtmultimedia5-dev
RUN apt-get install -y libqt5svg5-dev
RUN apt-get install -y libssl-dev make g++

COPY compile.sh /compile.sh

VOLUME /artifacts

ENTRYPOINT /compile.sh