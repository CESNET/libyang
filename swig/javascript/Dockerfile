FROM sysrepo/sysrepo-netopeer2

MAINTAINER mislav.novakovic@sartura.hr

# install node 7.x and npm
RUN \
      apt-get update && apt-get install -y \
      nodejs \
      npm \
      byacc \
      autoconf \
      yodl

# latest SWIG version with pull request
RUN \
      cd /opt/dev && \
      git clone https://github.com/swig/swig.git && \
      cd swig && \
      git fetch origin pull/236/head:javascript_sharedptr && \
      git checkout javascript_sharedptr && \
      ./autogen.sh && \
      ./configure --prefix=/usr && \
      make -j2 && \
      make install

# libyang
RUN \
      cd /opt/dev/libyang/build && \
      cmake -DENABLE_STATIC=ON -DGEN_LANGUAGE_BINDINGS=ON -DGEN_JAVASCRIPT_BINDINGS=ON -DGEN_PYTHON_BINDINGS=OFF -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE:String="Release" .. && \
      make -j2 && \
      make install && \
      cd javascript && \
      npm install --unsafe-perm && \
      node test.js && \
      ldconfig
