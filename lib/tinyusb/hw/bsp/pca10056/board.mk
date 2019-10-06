CFLAGS += \
	-DCFG_TUSB_MCU=OPT_MCU_NRF5X \
	-DNRF52840_XXAA \
	-mthumb \
	-Wno-error=undef \
	-Wno-error=cast-align \
	-mabi=aapcs \
	-mcpu=cortex-m4 \
	-mfloat-abi=hard \
	-mfpu=fpv4-sp-d16

# All source paths should be relative to the top level.
LD_FILE = hw/mcu/nordic/nrfx/mdk/nrf52840_xxaa.ld

LDFLAGS += -L$(TOP)/hw/mcu/nordic/nrfx/mdk

SRC_C += \
	hw/mcu/nordic/nrfx/drivers/src/nrfx_power.c \
	hw/mcu/nordic/nrfx/mdk/system_nrf52840.c \

INC += \
	-I$(TOP)/hw/cmsis/Include \
	-I$(TOP)/hw/mcu/nordic \
	-I$(TOP)/hw/mcu/nordic/nrfx \
	-I$(TOP)/hw/mcu/nordic/nrfx/mdk \
	-I$(TOP)/hw/mcu/nordic/nrfx/hal \
	-I$(TOP)/hw/mcu/nordic/nrfx/drivers/include \

SRC_S += hw/mcu/nordic/nrfx/mdk/gcc_startup_nrf52840.S

ASFLAGS += -D__HEAP_SIZE=0
ASFLAGS += -DCONFIG_GPIO_AS_PINRESET
ASFLAGS += -DBLE_STACK_SUPPORT_REQD
ASFLAGS += -DSWI_DISABLE0
ASFLAGS += -DFLOAT_ABI_HARD
ASFLAGS += -DNRF52840_XXAA

VENDOR = nordic
CHIP_FAMILY = nrf5x

