
rm libgenesis.dylib
make

FILE=libgenesis.dylib
if [ -f "$FILE" ];then
    mv CMakeFiles generated/
    mv cmake_install.cmake generated/
    mv CMakeCache.txt generated/

    echo "Library created."
    cd streamlit
    streamlit run app.py
fi

