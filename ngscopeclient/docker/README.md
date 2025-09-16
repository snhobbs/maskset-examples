# Docker installation of ngscopeclient 

+ [Main Repo](https://github.com/ngscopeclient/ngscope-apps)
+ [HAL Repo](https://github.com/ngscopeclient/scopehal)
+ [Docs](https://www.ngscopeclient.org/manual/GettingStarted.html)


## Docker install 

### Build

```sh 
docker build -f Dockerfile  --build-arg USERNAME=$(whoami) -t ngscope .
```

### Running 
```sh 
docker run -it --rm \                                                     
  -e DISPLAY=$DISPLAY   --device /dev/dri --group-add video \
  -v /tmp/.X11-unix:/tmp/.X11-unix -v ${HOME}:${HOME} \
  --name ngscope ngscope sudo ngscopeclient
```
