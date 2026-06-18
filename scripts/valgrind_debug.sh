#!/bin/bash

valgrind \
  --leak-check=full \
  --show-leak-kinds=definite,indirect \
  --errors-for-leak-kinds=definite,indirect --suppressions=./configs/valgrind.supp "$@"
