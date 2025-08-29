#FROM antmicro/renode:nightly-dotnet
FROM antmicro/renode:1.15.3

# Install necessary dependencies for X11 and GTK, needed of nightly build
#RUN apt-get update && \
#    apt-get install -y libgtk-3-0 libglib2.0-0

# Set environment variables for X11 forwarding
ENV DOTNET_BUNDLE_EXTRACT_BASE_DIR=/home/user
WORKDIR /home/user

# Set the default command to run bash (this can be customized later as needed)
CMD ["renode"]
