FROM ubuntu:20.04
RUN apt-get --yes -qq update \
    && apt-get --yes -qq upgrade \
    && DEBIAN_FRONTEND=noninteractive apt-get --yes -qq install \
               git cmake python \
               libfmt-dev libspdlog-dev nlohmann-json3-dev \
               libgrpc++-dev protobuf-compiler-grpc libssl-dev \
               build-essential pkg-config \
    && apt-get --yes -qq clean
RUN mkdir -p /home/build && mkdir -p /home/src && mkdir -p /home/build/packages
COPY . /home/src
WORKDIR /home/build
RUN cmake -DENABLE_UNIT_TESTS=OFF -DENABLE_FUNC_TESTS=OFF ../src && \
    make -j5 all && \
    make package && \
    mv bear-* packages/.
