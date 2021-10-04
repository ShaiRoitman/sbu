FROM ubuntu:18.04

RUN  apt-get update 
RUN  apt-get install -y wget gcc cmake g++
RUN  apt-get install -y libboost-all-dev
RUN  apt-get install -y libcurl4-openssl-dev zlib1g-dev
RUN  apt-get install -y git
RUN  rm -rf /var/lib/apt/lists/*

WORKDIR /
RUN mkdir git
WORKDIR /git
RUN git clone https://github.com/openssl/openssl
RUN git clone -b master https://github.com/pocoproject/poco.git
RUN git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp.git
RUN git clone https://github.com/SRombauts/SQLiteCpp
RUN git clone https://github.com/alex85k/sqlite3-cmake.git

COPY SQLite3.CMakeLists.txt /git/sqlite3-cmake/CMakeLists.txt
COPY aws_compiler_settings.cmake /git/aws-sdk-cpp/cmake/compiler_settings.cmake

ENV WORKING_CPU 4
ENV OPENSSL_ROOT_DIR /git/openssl


WORKDIR /git/openssl
RUN ./config
RUN make -j $WORKING_CPU
RUN make install


RUN mkdir /git/aws-sdk-cpp/release
WORKDIR /git/aws-sdk-cpp/release
RUN cmake -DBUILD_ONLY="s3;core;transfer" ..
RUN make -j $WORKING_CPU
RUN make install


RUN mkdir /git/SQLiteCpp/release
WORKDIR /git/SQLiteCpp/release
RUN cmake ..
RUN make -j $WORKING_CPU
RUN make install

WORKDIR /git/sqlite3-cmake
RUN cmake -DSQLITE_ENABLE_COLUMN_METADATA=Y .
RUN make install

RUN mkdir /git/poco/releaseBuild
WORKDIR /git/poco/releaseBuild
RUN cmake ..
RUN make -j $WORKING_CPU
RUN make install 

WORKDIR /git
RUN git clone http://github.com/ShaiRoitman/CompileToCppVar
WORKDIR /git/CompileToCppVar
RUN cmake .
RUN make -j $WORKING_CPU

WORKDIR /sbu

CMD /bin/bash

