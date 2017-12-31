#ifndef _UTIL_H
#define _UTIL_H

// Bit manipulation: Set / clear / get / toggle bit
#define sbi(ADDR, BIT) ((ADDR |=  (1<<BIT)))
#define cbi(ADDR, BIT) ((ADDR &= ~(1<<BIT)))
#define gbi(ADDR, BIT) ((ADDR &   (1<<BIT)))
#define tbi(ADDR, BIT) ((ADDR ^=  (1<<BIT)))

// For debugging:
void i2c_scanbus();

#endif
