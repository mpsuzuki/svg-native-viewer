# Building libsvgnative_wrap on GNU/Linux

This is not official document.

# scope of this document

this document is a memo how to build libsvgnative_wrap on GNU/Linux,
a wrapper of libSVGNativeViewerLib for C clients.

# how to build

```
cd svgnative
cmake -BBuild/linux-capi -H. -DSKIA=ON -DCAIRO=ON -DQT=ON -DBUILD_SHARED_LIBS=ON
make -CBuild/linux-capi
Build/linux-capi/example/c-api/testCAPI test/clipping.svg Build/linux-capi/example/c-api/clipping.png
```

At present, libsvgnative_wrap does not create the graphic contexts
by itself, the client should create by themselves. The "testCAPI"
creates Cairo recording surface and take its snapshot onto PNG.
Thus, libSVGNativeViewerLib should enable its Cairo port.
