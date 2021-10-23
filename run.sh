
rm libgenesis.dylib
rm -rf generated/CMakeFiles
rm generated/cmake_install.cmake
rm generated/CMakeCache.txt
rm generated/Makefile

make -j4

FILE=libgenesis.dylib
if [ -f "$FILE" ];then
    mv CMakeFiles generated/
    mv cmake_install.cmake generated/
    mv CMakeCache.txt generated/
    mv Makefile generated/

    echo "Library created."
    cd streamlit
    streamlit run app.py
fi

