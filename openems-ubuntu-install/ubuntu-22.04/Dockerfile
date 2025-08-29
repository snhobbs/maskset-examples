FROM ubuntu:22.04

WORKDIR /home/root
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
     apt-get upgrade -y && \
     apt-get install -y build-essential cmake git libhdf5-dev libvtk9-dev \
                     libboost-all-dev libcgal-dev libtinyxml-dev \
                     python3-numpy python3-matplotlib cython3 python3-h5py \
                     python3-venv

RUN git clone --recursive https://github.com/thliebig/openEMS-Project
RUN python3 -m venv venv && . venv/bin/activate && pip install numpy h5py matplotlib cython setuptools
RUN cd openEMS-Project && ./update_openEMS.sh ~/.local --python || cat *.log
RUN ./build_python.sh ~/.local/ || cat *.log
