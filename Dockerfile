FROM antmicro/renode:nightly-dotnet

# Install necessary dependencies for X11 and GTK
RUN apt-get update && \
    apt-get install -y libgtk-3-0 libglib2.0-0

# Set environment variables for X11 forwarding
ENV DOTNET_BUNDLE_EXTRACT_BASE_DIR=/home/user
WORKDIR /home/user

# Mount the X11 socket and the current directory (from the host) into the container
VOLUME ["/tmp/.X11-unix"]

# Set the default command to run bash (this can be customized later as needed)
CMD ["renode"]
