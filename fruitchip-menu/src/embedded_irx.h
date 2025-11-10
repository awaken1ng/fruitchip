#pragma once

#include <stdint.h>

// ppctty dependencies: loadcore, stdio, sysclib, ioman, thsemap
extern uint8_t PPCTTY_IRX[];
extern const uint32_t PPCTTY_IRX_SIZE;

extern uint8_t FWFS_IRX[];
extern const uint32_t FWFS_IRX_SIZE;

// padman dependencies: loadcore, intrman, thbase, thevent, sifman, sifcmd, stdio, sysclib, vblank, sio2man, sysmem
extern uint8_t FREEPAD_IRX[];
extern const uint32_t FREEPAD_IRX_SIZE;

// bdm dependencies: loadcore, stdio, sysclib, sysmem, thbase, thevent
extern uint8_t BDM_IRX[];
extern const uint32_t BDM_IRX_SIZE;

// bdmfs_fatfs dependencies: bdm, cdvdman, intrman, ioman, stdio, sysclib, sysmem, thsemap
extern uint8_t BDMFS_FATFS_IRX[];
extern const uint32_t BDMFS_FATFS_IRX_SIZE;

// usbd dependencies: sysmem, loadcore, intrman, stdio, thbase, thevent, thsemap, sysclib
extern uint8_t USBD_IRX[];
extern const uint32_t USBD_IRX_SIZE;

// usbmass_bd dependencies: bdm, stdio, sysclib, thbase, thsemap, usbd
extern uint8_t USBMASS_BD_IRX[];
extern const uint32_t USBMASS_BD_IRX_SIZE;
