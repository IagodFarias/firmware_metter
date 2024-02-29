#ifndef __CORE_BASE_H_
#define __CORE_BASE_H_

#ifndef BASE_ADDR_WB_CPU_MODS
#define BASE_ADDR_WB_CPU_MODS                      (0x0000U)
#endif
#ifndef BASE_ADDR_WB_DM1_PERI
#define BASE_ADDR_WB_DM1_PERI                      (0x0400U)
#endif
#ifndef BASE_ADDR_WB_DM0_PERI
#define BASE_ADDR_WB_DM0_PERI                      (0x0800U)
#endif
#ifndef BASE_ADDR_ROM
#define BASE_ADDR_ROM                              (0x0000U)
#endif
#ifndef BASE_ADDR_SWTIMER
#define BASE_ADDR_SWTIMER                          (0x0080U)
#endif
#ifndef BASE_ADDR_H430_MUL
#define BASE_ADDR_H430_MUL                         (0x0100U)
#endif
#ifndef BASE_ADDR_FPU
#define BASE_ADDR_FPU                              (0x0180U)
#endif
#ifndef BASE_ADDR_VIC
#define BASE_ADDR_VIC                              (0x0200U)
#endif
#ifndef BASE_ADDR_WDOG1
#define BASE_ADDR_WDOG1                            (0x0280U)
#endif
#ifndef BASE_ADDR_WDOG0L
#define BASE_ADDR_WDOG0L                           (0x0D00U)
#endif
#ifndef BASE_ADDR_WDOG0H
#define BASE_ADDR_WDOG0H                           (0x0D80U)
#endif
#ifndef BASE_ADDR_SPI
#define BASE_ADDR_SPI                              (0x0400U)
#endif
#ifndef BASE_ADDR_UART0
#define BASE_ADDR_UART0                            (0x0440U)
#endif
#ifndef BASE_ADDR_UART1
#define BASE_ADDR_UART1                            (0x0480U)
#endif
#ifndef BASE_ADDR_I2C
#define BASE_ADDR_I2C                              (0x04C0U)
#endif
#ifndef BASE_ADDR_I2C_SLAVE
#define BASE_ADDR_I2C_SLAVE                        (0x0500U)
#endif
#ifndef BASE_ADDR_FLASH_CTRL
#define BASE_ADDR_FLASH_CTRL                       (0x0580U)
#endif
#ifndef BASE_ADDR_DMA_A0
#define BASE_ADDR_DMA_A0                           (0x05C0U)
#endif
#ifndef BASE_ADDR_DMA_A1
#define BASE_ADDR_DMA_A1                           (0x0600U)
#endif
#ifndef BASE_ADDR_DMA_B0
#define BASE_ADDR_DMA_B0                           (0x0640U)
#endif
#ifndef BASE_ADDR_DMA_B1
#define BASE_ADDR_DMA_B1                           (0x0680U)
#endif
#ifndef BASE_ADDR_IOMUX_CTRL
#define BASE_ADDR_IOMUX_CTRL                       (0x0800U)
#endif
#ifndef BASE_ADDR_SYS_STATE
#define BASE_ADDR_SYS_STATE                        (0x0880U)
#endif
#ifndef BASE_ADDR_RTC
#define BASE_ADDR_RTC                              (0x0900U)
#endif
#ifndef BASE_ADDR_PULSE
#define BASE_ADDR_PULSE                            (0x0980U)
#endif
#ifndef BASE_ADDR_PWM
#define BASE_ADDR_PWM                              (0x0A00U)
#endif
#ifndef BASE_ADDR_AD_CTRL
#define BASE_ADDR_AD_CTRL                          (0x0A80U)
#endif
#ifndef BASE_ADDR_UL_CTRL
#define BASE_ADDR_UL_CTRL                          (0x0B00U)
#endif
#ifndef BASE_ADDR_AG_CTRL
#define BASE_ADDR_AG_CTRL                          (0x0B80U)
#endif
#ifndef BASE_ADDR_TX_CTRL
#define BASE_ADDR_TX_CTRL                          (0x0C00U)
#endif
#ifndef BASE_ADDR_GPIO
#define BASE_ADDR_GPIO                             (0x0C80U)
#endif
#ifndef BASE_ADDR_DMEM
#define BASE_ADDR_DMEM                             (0x1000U)
#endif
#ifndef BASE_ADDR_IMEM
#define BASE_ADDR_IMEM                             (0x3000U)
#endif

#ifndef INST_SIZE_WB_CPU_MODS
#define INST_SIZE_WB_CPU_MODS                      (0x0400U)
#endif
#ifndef INST_SIZE_WB_DM1_PERI
#define INST_SIZE_WB_DM1_PERI                      (0x0400U)
#endif
#ifndef INST_SIZE_WB_DM0_PERI
#define INST_SIZE_WB_DM0_PERI                      (0x0800U)
#endif
#ifndef INST_SIZE_ROM
#define INST_SIZE_ROM                              (0x0080U)
#endif
#ifndef INST_SIZE_SWTIMER
#define INST_SIZE_SWTIMER                          (0x0080U)
#endif
#ifndef INST_SIZE_H430_MUL
#define INST_SIZE_H430_MUL                         (0x0080U)
#endif
#ifndef INST_SIZE_FPU
#define INST_SIZE_FPU                              (0x0080U)
#endif
#ifndef INST_SIZE_VIC
#define INST_SIZE_VIC                              (0x0080U)
#endif
#ifndef INST_SIZE_WDOG1
#define INST_SIZE_WDOG1                            (0x0080U)
#endif
#ifndef INST_SIZE_WDOG0L
#define INST_SIZE_WDOG0L                           (0x0080U)
#endif
#ifndef INST_SIZE_WDOG0H
#define INST_SIZE_WDOG0H                           (0x0080U)
#endif
#ifndef INST_SIZE_SPI
#define INST_SIZE_SPI                              (0x0040U)
#endif
#ifndef INST_SIZE_UART0
#define INST_SIZE_UART0                            (0x0040U)
#endif
#ifndef INST_SIZE_UART1
#define INST_SIZE_UART1                            (0x0040U)
#endif
#ifndef INST_SIZE_I2C
#define INST_SIZE_I2C                              (0x0040U)
#endif
#ifndef INST_SIZE_I2C_SLAVE
#define INST_SIZE_I2C_SLAVE                        (0x0040U)
#endif
#ifndef INST_SIZE_FLASH_CTRL
#define INST_SIZE_FLASH_CTRL                       (0x0040U)
#endif
#ifndef INST_SIZE_DMA_A0
#define INST_SIZE_DMA_A0                           (0x0040U)
#endif
#ifndef INST_SIZE_DMA_A1
#define INST_SIZE_DMA_A1                           (0x0040U)
#endif
#ifndef INST_SIZE_DMA_B0
#define INST_SIZE_DMA_B0                           (0x0040U)
#endif
#ifndef INST_SIZE_DMA_B1
#define INST_SIZE_DMA_B1                           (0x0040U)
#endif
#ifndef INST_SIZE_IOMUX_CTRL
#define INST_SIZE_IOMUX_CTRL                       (0x0080U)
#endif
#ifndef INST_SIZE_SYS_STATE
#define INST_SIZE_SYS_STATE                        (0x0080U)
#endif
#ifndef INST_SIZE_RTC
#define INST_SIZE_RTC                              (0x0080U)
#endif
#ifndef INST_SIZE_PULSE
#define INST_SIZE_PULSE                            (0x0080U)
#endif
#ifndef INST_SIZE_PWM
#define INST_SIZE_PWM                              (0x0080U)
#endif
#ifndef INST_SIZE_AD_CTRL
#define INST_SIZE_AD_CTRL                          (0x0080U)
#endif
#ifndef INST_SIZE_UL_CTRL
#define INST_SIZE_UL_CTRL                          (0x0080U)
#endif
#ifndef INST_SIZE_AG_CTRL
#define INST_SIZE_AG_CTRL                          (0x0080U)
#endif
#ifndef INST_SIZE_TX_CTRL
#define INST_SIZE_TX_CTRL                          (0x0080U)
#endif
#ifndef INST_SIZE_GPIO
#define INST_SIZE_GPIO                             (0x0080U)
#endif
#ifndef INST_SIZE_DMEM
#define INST_SIZE_DMEM                             (0x1400U)
#endif
#ifndef INST_SIZE_IMEM
#define INST_SIZE_IMEM                             (0xD000U)
#endif

#endif
