## Download GSL and compile
curl https://ftp.gnu.org/gnu/gsl/gsl-1.14.tar.gz > /tmp/gsl-1.14.tar.gz
cd /tmp
tar xzvf gsl-1.14.tar.gz 
cd gsl-1.14
./configure --prefix=/usr/local
make -j32
make install -j32

# Download Boost
wget -O /tmp/boost_1_55_0.tar.gz https://sourceforge.net/projects/boost/files/boost/1.55.0/boost_1_55_0.tar.gz/download
cd /tmp
tar xzvf boost_1_55_0.tar.gz
cd boost_1_55_0/
./bootstrap.sh --prefix=/usr/local
./b2
./b2 install

# TODO: Change the path of install boost via apt
# apt install libboost-all-dev -y

## Download GLPK and compile
curl https://ftp.gnu.org/gnu/glpk/glpk-4.65.tar.gz >  /tmp/glpk-4.65.tar.gz
cd /tmp
tar xzvf glpk-4.65.tar.gz
cd glpk-4.65
./configure --prefix=/usr/local --disable-dl --disable-odbc --disable-mysql --enable-static --without-gmp --without-zlib
make -j32
make install -j32

# Compile the mslib with GSL, Boost and GLPK
git clone https://github.com/2019ncov-epsilon/mslib $HOME/mslib
cd $HOME/mslib
echo 'export MSL_GSL=T' >> $HOME/.bashrc
echo 'export MSL_GLPK=T' >> $HOME/.bashrc
echo 'export MSL_BOOST=T' >> $HOME/.bashrc
echo 'export MSL_R=F' >> $HOME/.bashrc
echo 'export MSL_EXTERNAL_LIB_DIR="/usr/local/lib"' >> $HOME/.bashrc
source $HOME/.bashrc
make -j32