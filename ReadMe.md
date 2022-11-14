Building
==
Pre-requirements
--
gRPC should be installed as described in [official doc](https://grpc.io/docs/languages/cpp/quickstart/) 
and built with 
`-DgRPC_SSL_PROVIDER=package` 

Centos 7
--
Install freeswitch repo as described in [confluence](https://freeswitch.org/confluence/display/FREESWITCH/CentOS+7+and+RHEL+7), than  
```shell
sudo yum installl -y freeswitch-devel
sudo ln -s /usr/share/freeswitch/pkgconfig/freeswitch.pc /usr/lib64/pkgconfig/ 
```
You need to check correct path to your pkgconfig directory