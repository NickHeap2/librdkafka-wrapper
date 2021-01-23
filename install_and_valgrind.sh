cmake3 --build /mnt/d/workspaces/nickheap2/librdkafka-wrapper/out/build/WSL-Centos-Debug --target install
valgrind --tool=memcheck --leak-check=full --suppressions=valgrind.supp out/install/WSL-Centos-Debug/bin/consumer-test
