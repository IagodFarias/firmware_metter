#ifndef HAVE_BASE_H
#define HAVE_BASE_H

#include "types.h"

/* ======================================================================= */
/* Pragmas for writing to registers.                                       */
/* ======================================================================= */

#ifdef DPI

	void     WRITE_REG_8   (uint16_t addr, uint8_t  data);
	uint8_t  READ_REG_U8   (uint16_t addr);
	int8_t   READ_REG_S8   (uint16_t addr);
	void     BIS_REG_8     (uint16_t addr, uint8_t  data);
	void     BIC_REG_8     (uint16_t addr, uint8_t  data);

	void     WRITE_REG_16  (uint16_t addr, uint16_t data);
	uint16_t READ_REG_U16  (uint16_t addr);
	int16_t  READ_REG_S16  (uint16_t addr);
	void     BIS_REG_16    (uint16_t addr, uint16_t data);
	void     BIC_REG_16    (uint16_t addr, uint16_t data);

	void     WRITE_REG_32  (uint16_t addr, uint32_t data);
	uint32_t READ_REG_U32  (uint16_t addr);
	int32_t  READ_REG_S32  (uint16_t addr);
	void     BIS_REG_32    (uint16_t addr, uint32_t data);
	void     BIC_REG_32    (uint16_t addr, uint32_t data);
  
  void     WRITE_REG_64  (uint16_t addr, uint64_t data);
	uint64_t READ_REG_U64  (uint16_t addr);
	int64_t  READ_REG_S64  (uint16_t addr);
	void     BIS_REG_64    (uint16_t addr, uint64_t data);
	void     BIC_REG_64    (uint16_t addr, uint64_t data);

	void     WRITE_MEM_PTR (uint16_t addr, uintptr_t ptr);

#else

	#define WRITE_REG_8(addr,data)   (*(volatile uint8_t *)(addr) = (data))
	#define READ_REG_U8(addr)        ((uint8_t)(*(volatile uint8_t *)(addr)))
	#define READ_REG_S8(addr)        (( int8_t)(*(volatile  int8_t *)(addr)))
	#define BIS_REG_8(addr,data)     (*(volatile uint8_t *)(addr) |=  (uint8_t)(data))
	#define BIC_REG_8(addr,data)     (*(volatile uint8_t *)(addr) &= ~(uint8_t)(data))

	#define WRITE_REG_16(addr,data)  (*(volatile uint16_t *)(addr) = (data))
	#define READ_REG_U16(addr)       ((uint16_t)(*(volatile uint16_t *)(addr)))
	#define READ_REG_S16(addr)       (( int16_t)(*(volatile  int16_t *)(addr)))
	#define BIS_REG_16(addr,data)    (*(volatile uint16_t *)(addr) |=  (uint16_t)(data))
	#define BIC_REG_16(addr,data)    (*(volatile uint16_t *)(addr) &= ~(uint16_t)(data))

	#define WRITE_REG_32(addr,data)  (*(volatile uint32_t *)(addr) = (data))
	#define READ_REG_U32(addr)       ((uint32_t)(*(volatile uint32_t *)(addr)))
	#define READ_REG_S32(addr)       (( int32_t)(*(volatile  int32_t *)(addr)))
	#define BIS_REG_32(addr,data)    (*(volatile uint32_t *)(addr) |=  (uint32_t)(data))
	#define BIC_REG_32(addr,data)    (*(volatile uint32_t *)(addr) &= ~(uint32_t)(data))
  
  #define WRITE_REG_64(addr,data)  (*(volatile uint64_t *)(addr) = (data))
	#define READ_REG_U64(addr)       ((uint64_t)(*(volatile uint64_t *)(addr)))
	#define READ_REG_S64(addr)       (( int64_t)(*(volatile  int64_t *)(addr)))
	#define BIS_REG_64(addr,data)    (*(volatile uint64_t *)(addr) |=  (uint64_t)(data))
	#define BIC_REG_64(addr,data)    (*(volatile uint64_t *)(addr) &= ~(uint64_t)(data))

	#define WRITE_MEM_PTR(addr,ptr)  (*(volatile uintptr_t *)(addr) = (ptr))

#endif

#endif /* HAVE_BASE_H */

