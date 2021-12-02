#########################################################################################
# Micro XRCE-DDS Docker
#########################################################################################

# Build stage
FROM ubuntu AS build
ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /root

# Essentials
RUN apt-get update
RUN apt-get install -y \
            software-properties-common \
            build-essential \
            cmake \
            git

# Java
RUN apt install -y openjdk-8-jdk
ENV JAVA_HOME "/usr/lib/jvm/java-8-openjdk-amd64/"

# Gradle
RUN apt-get install -y gradle

RUN apt-get clean

# Prepare Micro XRCE-DDS workspace
RUN mkdir -p /uxrce/build
ADD . /uxrce/

# Build Micro XRCE-DDS and install
RUN cd /uxrce/build && \
    cmake \
        -DCMAKE_INSTALL_PREFIX=../install \
        -DUXRCE_BUILD_EXAMPLES=ON \
        -DUXRCE_BUILD_AGENT_EXECUTABLE=ON \
        .. &&\
    make -j $(nproc) && make install

# Prepare Micro XRCE-DDS artifacts
RUN cd /uxrce && \
    tar -czvf install.tar.gz  -C install .

# Final user image
FROM ubuntu
WORKDIR /root

# Copy Micro XRCE-DDS build artifacts
COPY --from=build /uxrce/install.tar.gz  /usr/local/
RUN tar -xzvf /usr/local/install.tar.gz -C /usr/local/ &&\
    rm /usr/local/install.tar.gz

RUN apt update \
&&  apt install -y wget \
&&  rm -rf /var/lib/apt/lists/*
RUN wget https://raw.githubusercontent.com/eProsima/Micro-XRCE-DDS-Agent/master/agent.refs

RUN ldconfig