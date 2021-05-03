FROM centos:centos7.9.2009

# install cmake3
RUN curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/l/libzstd-1.4.7-1.el7.x86_64.rpm -O \
 && curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/r/rhash-1.3.4-2.el7.x86_64.rpm -O \
 && curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/l/libuv-1.40.0-1.el7.x86_64.rpm -O \
 && curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/cmake3-data-3.17.5-1.el7.noarch.rpm -O \
 && curl https://download-ib01.fedoraproject.org/pub/epel/7/x86_64/Packages/c/cmake3-3.17.5-1.el7.x86_64.rpm -O \
 && yum localinstall -y libzstd-1.4.7-1.el7.x86_64.rpm rhash-1.3.4-2.el7.x86_64.rpm libuv-1.40.0-1.el7.x86_64.rpm cmake3-3.17.5-1.el7.x86_64.rpm cmake3-data-3.17.5-1.el7.noarch.rpm \
 && rm *.rpm \
 && yum clean all \
 && rm -rf /var/cache/yum
 
 # gcc and librdkafka and openssl dependency
RUN yum install -y openssl gcc cmake librdkafka-devel
 #&& yum clean all \
 #&& rm -rf /var/cache/yum

RUN mkdir /opt/cmake

COPY *.* /opt/cmake/

WORKDIR /opt/cmake/

RUN cmake3 -G "Unix Makefiles"  -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_INSTALL_PREFIX:PATH="/opt/cmake/out/install"  --build /opt/cmake/
RUN cmake3 --build . --target install

COPY qa-dhlparcel-co-uk.cer /opt/cmake/out/install/WSL-Centos-Debug/bin/

#add confluent repos
COPY confluent.repo /etc/yum.repos.d/

RUN yum erase -y librdkafka librdkafka-devel

#install rdkafka and serdes
RUN yum install -y confluent-libserdes.x86_64 librdkafka1
