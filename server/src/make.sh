
#The main file
g++ -g -Wall -c gcs.cpp

#Invoking: GCC C++ Linker
g++ -o "GCServer" -g -Wall -std=gnu99 gcs.o -lboost_thread -lavutil -lavformat -lavcodec -lavdevice -lswscale
