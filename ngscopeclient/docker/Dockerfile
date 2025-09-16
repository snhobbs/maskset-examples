# Base image
FROM debian:13.1
ENV DEBIAN_FRONTEND=noninteractive 
RUN apt-get update && apt-get install -y \
    git wget curl cmake pkgconf build-essential ninja-build \
    libx11-dev x11-apps libgtk-3-dev libsigc++-2.0-dev \
    libyaml-cpp-dev catch2 libglfw3-dev \
    libhidapi-dev liblxi-dev libtirpc-dev \
    libvulkan-dev glslang-tools spirv-tools

RUN apt-get upgrade -y
ENV PKG=ngscopeclient-debian-13-amd64-v0.1.deb
RUN wget https://github.com/ngscopeclient/scopehal-apps/releases/download/v0.1/${PKG}
RUN dpkg -i ${PKG} || apt-get -f install -y

# Set arguments for user creation (replace defaults with your own if needed)
ARG USERNAME=${USER}
ARG USER_UID=1000
ARG USER_GID=$USER_UID

# Install sudo
RUN apt-get update && apt-get install -y sudo

# Create the user with matching UID/GID and add to sudo
RUN groupadd --gid $USER_GID $USERNAME \
    && useradd --uid $USER_UID --gid $USER_GID -m $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# Switch to the new user
USER $USERNAME
WORKDIR /home/$USERNAME
