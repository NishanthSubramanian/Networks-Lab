sudo apt-get install gcc g++ python python-dev python-setuptools git mercurial qt5-default python-pygraphviz python-kiwi python-pygoocanvas libgoocanvas-dev ipython gir1.2-goocanvas-2.0 python-gi python-gi-cairo python-pygraphviz python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython ipython3 openmpi-bin openmpi-common openmpi-doc libopenmpi-dev autoconf cvs bzr unrargdb valgrind uncrustify doxygen graphviz imagemagick texlive texlive-extra-utils texlive-latex-extra texlive-font-utils texlive-lang-portuguese dvipng latexmk python-sphinx dia gsl-bin libgsl-dev libgsl23 libgslcblas0 tcpdump sqlite sqlite3 libsqlite3-dev  libxml2 libxml2-dev cmake libc6-dev libc6-dev-i386 libclang-6.0-dev llvm-6.0-dev automake pip libgtk2.0-0 libgtk2.0-dev vtun lxc libboost-signals-dev libboost-filesystem-dev;

hg clone http://code.nsnam.org/ns-3-allinone
cd ns-3-allinone
./download.py -n ns-3-dev

./build.py
cd ns-3-dev
./waf configure --enable-tests --enable-examples -d debug
