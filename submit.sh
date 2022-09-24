#!/bin/bash

echo "zip project1-submission.zip"

zip project1-submission.zip \
    src/include/buffer/lru_replacer.h \
    src/buffer/lru_replacer.cpp \
    src/include/buffer/buffer_pool_manager_instance.h \
    src/buffer/buffer_pool_manager_instance.cpp \
    src/include/buffer/parallel_buffer_pool_manager.h \
    src/buffer/parallel_buffer_pool_manager.cpp

echo ""

echo "move from Fedora to Windows codefiles/"

mv project1-submission.zip /mnt/hgfs/

