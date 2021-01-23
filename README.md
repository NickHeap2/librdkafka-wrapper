# oekafka-wrapper
C wrapper for librdkafka for OpenEdge

choco install nuget.commandline
Install from visual studio setup:
	C++ CMake tools for Windows 
	c++ CMake tools for Linux
	C++ core features
	C++ for Linux Development

Centos 7 WSL image
	yum -y update
	yum install -y openssl gcc cmake3 librdkafka-devel
    curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/l/libzstd-1.4.7-1.el7.x86_64.rpm -O
    curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/r/rhash-1.3.4-2.el7.x86_64.rpm -O
    curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/l/libuv-1.40.0-1.el7.x86_64.rpm -O
    curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/cmake3-data-3.17.5-1.el7.noarch.rpm -O
    curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/cmake3-3.17.5-1.el7.x86_64.rpm -O
    yum localinstall -y libzstd-1.4.7-1.el7.x86_64.rpm rhash-1.3.4-2.el7.x86_64.rpm libuv-1.40.0-1.el7.x86_64.rpm cmake3-3.17.5-1.el7.x86_64.rpm cmake3-data-3.17.5-1.el7.noarch.rpm

	copy confluent.repo /etc/yum.repo.d/
	yum install -y confluent-libserdes-devel.x86_64
