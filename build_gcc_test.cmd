g++ -O3 -msse2 -std=c++11 -Wall -Wextra -Wno-comment -s -static ^
  -I src ^
  test/test.cpp ^
  -o bin/test_gcc.exe

"bin/test_gcc.exe"
pause