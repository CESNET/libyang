FROM sysrepo/sysrepo-netopeer2:arch_devel

MAINTAINER mislav.novakovic@sartura.hr

# install node 7.x and npm
RUN \
      pacman -S --noconfirm \
      nodejs \
      npm

# latest SWIG version with pull request
RUN \
      cd /opt/dev && \
      git clone https://github.com/swig/swig.git && \
      cd swig && \
      git fetch origin git fetch origin pull/236/head:javascript_sharedptr && \
      git checkout javascript_sharedptr && \
      ./autogen.sh && \
      ./configure --prefix=/usr && \
      make -j2 && \
      make install

# libyang
RUN \
      cd /opt/dev/libyang/build && \
      cmake -DJAVASCRIPT_BINDING=ON -DCMAKE_INSTALL_PREFIX=/usr -DCMAKE_BUILD_TYPE:String="Debug" .. && \
      make -j2 && \
      make install && \
      cd javascript && \
      npm install --unsafe-perm && \
      node test.js && \
      ldconfig
