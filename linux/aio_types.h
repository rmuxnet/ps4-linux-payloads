/*
 * aio_types.h — Izoliuoti kernelio stiliaus sveikųjų skaičių tipai.
 *
 * SĄMONINGAI neįtraukia jokių POSIX ar standartinių antraščių. Tai garantuoja,
 * kad nebus jokių typedef konfliktų, kai AIO payload kompiliuojamas su
 * FreeBSD sys/types.h antraštėmis, kurios jau yra include kelyje. 
 * NET NXJ NETURETU VEIKTI BLET NES KAZKAS KURWA UNIFORMINIU BUDU NEGALI SURASYTI JOBANU KERNELIO OFFSETU
 */

#ifndef AIO_TYPES_H
#define AIO_TYPES_H

typedef unsigned char       u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

#endif /* AIO_TYPES_H */
