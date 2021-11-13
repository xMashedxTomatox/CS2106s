CC := gcc
CFLAGS := -g -std=c18 -O2 -Wall -Wextra -Werror -Wno-error=unused-parameter

all: test_mem_stress workload_wraddr workload_readonly workload_rdfile workload_wrfile test_file_huge test_file_multiread test_file_multiwrite test_multiregion_overflow test_multiregion_write gigabyte_test test_peakmem

workload_readonly: workload_readonly.o userswap.o userswap.h

workload_wraddr: workload_wraddr.o userswap.o userswap.h

workload_rdfile: workload_rdfile.o userswap.o userswap.h

workload_wrfile: workload_wrfile.o userswap.o userswap.h

test_file_huge: test_file_huge.o userswap.o userswap.h

test_file_multiread: test_file_multiread.o userswap.o userswap.h

test_file_multiwrite: test_file_multiwrite.o userswap.o userswap.h

test_multiregion_overflow: test_multiregion_overflow.o userswap.o userswap.h

test_multiregion_write: test_multiregion_write.o userswap.o userswap.h

gigabyte_test: gigabyte_test.o userswap.o userswap.h

test_peakmem: test_peakmem.o userswap.o userswap.h

test_mem_stress: test_mem_stress.o userswap.o userswap.h

clean:
	rm -f *.o workload_wraddr workload_readonly workload_rdfile workload_wrfile

.PHONY: all clean
