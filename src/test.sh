#!/bin/bash
echo "Running test.sh"

module=.
dir=test
number=1
test() {
  ${module}/dm510_load
  echo "Test $number : $1 : $2"
  ( cd $dir && eval $2)
  echo ""
  let number++
  ${module}/dm510_unload
}

#test "Blocked reading from empty buffer" "empty_block.out"
test "Non-Blocked reading from empty buffer" "./empty_non_block.out"

test "Writing and reading from same apllication" "./readwrite.out 'Hello :)'"
test "Registrering too many writers" "./too_many_writers.out"

test "Registrering too many readers" "./too_many_readers.out"
#test "Blocked writing to a full buffer" "fullbuffer_blocking.out"

test "Non-Blocked writing to a full buffer" "./fullbuffer_non_blocking.out"
test "Changeing buffer size" "./change_buffer_size.out 2048"

test "Changeing buffer size of non-empty buffer" "echo 'Hello :)' > /dev/dm510-0 && ./change_buffer_size.out 5"
test "I/O from concurrent applications" "./moduletest.out 10"
