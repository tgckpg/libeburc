#pragma once

/*
 * Test whether `off_t' represents a large integer.
 */
#define off_t_is_large \
    ((((off_t) 1 << 41) + ((off_t) 1 << 40) + 1) % 9999991 == 7852006)
