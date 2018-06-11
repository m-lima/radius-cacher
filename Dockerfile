FROM ubuntu:bionic

WORKDIR /opt/radius-cacher

RUN apt-get update && \
    apt-get install -y \
      g++ \
      cmake \
      libmemcached-dev \
      git

RUN git clone https://github.com/m-lima/radius-cacher.git source && \
    cd source && \
    git submodule update --init --recursive && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release -DRC_DISABLE_CACHE_OPERATIONS .. && \
    make && \
    mv radius-cacher /opt/radius-cacher/. && \
    cd /opt/radius-cacher && \
    rm -rf source

FROM ubuntu:bionic

WORKDIR /opt/radius-cacher

RUN apt-get update && \
    apt-get install -y \
      libmemcached11 && \
    apt-get clean

COPY --from=0 /opt/radius-cacher/radius-cacher /opt/radius-cacher/radius-cacher

RUN echo HOST=10.80.15.181 > /etc/radius-cacher/cache.conf

EXPOSE 1813/udp

CMD ./radius-cacher

