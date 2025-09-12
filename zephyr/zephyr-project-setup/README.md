# Bare Bones Zephyr Project Setup 
+ Enables MCUBoot and builds both images
+ Uses Manifest files and a west workspace
+ app is based on zephyr's samples/sysbuild/with_mcuboot


```sh 
python3 -m venv venv 
source venv/bin/activate
pip install west 
cd app
west init -l .
west update
west build --board nrf52840dk/nrf52840 --build-dir build --sysbuild -- -GNinja
```
