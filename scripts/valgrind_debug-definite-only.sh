#!/bin/bash

valgrind \
  --leak-check=full \
  --show-leak-kinds=definite \
  --errors-for-leak-kinds=definite --suppressions=./configs/valgrind.supp $1
