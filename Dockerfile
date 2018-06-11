FROM ubuntu:bionic

WORKDIR /opt/radius-cacher

RUN apt-get update && \
    apt-get install -y \
      g++ \
      cmake \
      libmemcached-dev \
      git

COPY . .

RUN rm -rf build && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make

# RUN git clone https://github.com/m-lima/radius-cacher.git source && \
#     cd source && \
#     git submodule update --init --recursive && \
#     mkdir build && \
#     cd build && \
#     cmake -DCMAKE_BUILD_TYPE=Release .. && \
#     make && \
#     mv radius-cacher /opt/radius-cacher/. && \
#     cd /opt/radius-cacher && \
#     rm -rf source

FROM ubuntu:bionic

WORKDIR /opt/radius-cacher

RUN apt-get update && \
    apt-get install -y \
      libmemcached11 && \
    apt-get clean

#COPY --from=0 /opt/radius-cacher/radius-cacher /opt/radius-cacher/radius-cacher
COPY --from=0 /opt/radius-cacher/build/radius-cacher /opt/radius-cacher/radius-cacher

RUN mkdir /etc/radius-cacher && \
    echo HOST=10.80.15.181 > /etc/radius-cacher/cache.conf

EXPOSE 1813/udp

CMD ./radius-cacher

