FROM ubuntu:20.04

WORKDIR /final_project/freamwork
RUN apt-get update


# Install necessary dependencies
RUN apt-get install -y g++

# Copy your C++ source code to the container
COPY /include/priority_queue.hpp .
COPY /include/waitable_queue.hpp .
COPY /include/utils.hpp .
COPY /test/waitable_queue_test.cpp .


# Compile the C++ program
RUN g++ -std=c++11 -g -I ./include waitable_queue_test.cpp -pthread -o waitable_queue.out

CMD ["./waitable_queue.out"]