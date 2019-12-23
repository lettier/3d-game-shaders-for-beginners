git clone https://github.com/panda3d/panda3d
# xcode-select --install # uncomment this if you don't have Command Line Tools installedls
export LIBRARY_PATH=$LIBRARY_PATH:/usr/local/opt/openssl/lib
cd panda3d
python3 makepanda/makepanda.py --everything --no-python
ln -s demo/panda3d/built/include ../../include
# ln -s demo/panda3d/built/lib ../../lib # seems like this will lead to shader errors
cd ..
sed s/mill-scene\"/mill-scene.egg\"/g src/main.cxx | tee src/main.tmp.cxx
mv src/main.tmp.cxx src/main.cxx
g++ -c src/main.cxx -Ipanda3d/built/include -o main.o -std=c++14 -O2
g++ main.o -o main -Lpanda3d/built/lib -lpanda -lp3framework -lpandafx -lpandaexpress -lp3dtoolconfig -lp3dtool -lp3direct -lpthread -lpandaegg -lp3openal_audio -lpandagl
mkdir ../lib
cd panda3d/built/lib
cp -r libp3direct.1.11.dylib libp3framework.1.11.dylib libpandafx.1.11.dylib libpanda.1.11.dylib libp3dtool.1.11.dylib libpandaegg.1.11.dylib libpandagl.a libp3dtoolconfig.1.11.dylib libpandaexpress.1.11.dylib libp3openal_audio.dylib libpandagl.dylib ../../../../lib
