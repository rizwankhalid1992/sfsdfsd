void setup() {
  // put your setup code here, to run once:

}

void loop() {
  // put your main code here, to run repeatedly:

}




/***
 *** The example has no copyright and can be used by anyone.
 *** The following example is based on Device File Package
 *** required to compile the macro definitions used.
 *** The Device File Package is available by downloading Atmel Studio 7.
 ***/
 
/***
 *** In this example The SERCOM0 is configured as SPI to work with DMA in close Loop.
 *** the SPI Frequency (Baud Rate) will be OSC16M Freq/2
 *** Several functions are described to be reused in concrete example
 ***/
 
/*** Define SPI Channels ***/
#define SPI_TX_CHANNEL      1
#define SPI_RX_CHANNEL      0
 
/*** Create DMA descriptor in global variable (128-bit aligned) ***/
volatile __attribute__((__aligned__(128))) DmacDescriptor descriptor_SPI[2];
volatile __attribute__((__aligned__(128))) DmacDescriptor write_back_descriptor_SPI[2];
 
/*** SPI Output Data Buffer ***/
 
const uint8_t spi_output_data[SPI_DATA_SIZE+1] = 
{
 0xd5, 0xed, 0x50, 0x1c, 0x9e, 0xf0, 0x65, 0xb6,
 0xed, 0x85, 0xf2, 0x36, 0x8e, 0x3e, 0x11, 0x12,
 0x3d, 0x34, 0xe9, 0x66, 0x17, 0x50, 0x46, 0x62,
 0xad, 0xfd, 0xe3, 0x43, 0x40, 0x85, 0x0d, 0x13,
 0xbe, 0xd3, 0xf5, 0xb9, 0x8f, 0x18, 0x5b, 0xe5,
 0xd3, 0xed, 0x6e, 0xc3, 0x7e, 0xe6, 0x3c, 0x2b,
 0x89, 0xb1, 0x50, 0x36, 0xae, 0x9b, 0x2b, 0x11,
 0x49, 0xe3, 0xf4, 0x50, 0x55, 0xfe, 0x90, 0xae,
 0x7f, 0xb3, 0x17, 0x2a, 0xdc, 0x6d, 0x08, 0xe9,
 0xf7, 0x9a, 0x57, 0xf9, 0x78, 0xa2, 0x9c, 0xd8,
 0x5a, 0x30, 0xc7, 0x9d, 0x31, 0xb9, 0xce, 0x85,
 0x14, 0x98, 0x2c, 0x8d, 0xb8, 0x04, 0xf5, 0x0d,
 0x16, 0xf1, 0x26, 0xea, 0x15, 0x5e, 0x83, 0xd1,
 0x07, 0x35, 0x60, 0x5b, 0x87, 0xde, 0x8b, 0x33,
 0x6f, 0x73, 0x53, 0xbf, 0xa3, 0x7a, 0xd7, 0x2f,
 0xf5, 0x81, 0xf8, 0x10, 0xcd, 0xdd, 0xc2, 0xb6,
 0x00
};
 
/*** SPI RX Buffer ***/
extern uint8_t spi_rx_buffer[SPI_DATA_SIZE];
 
/*************************************************/
/*** DMA RX Descriptor Initialization step by step
 *** - Validate the Descriptor
 *** - No Event out generated by the DMA
 *** - Channel will be disabled if it is the last block 
 ***   transfer in the transaction and block interrupt
 *** - 8-bit bus transfer (BYTE)
 *** - No increment from the source address (SRCINC=0)
 *** - Enable Increment from the destination address (DSTINC=1)
 *** - Step size settings apply to the Destination address
 *** - Step Size => Next ADDR = ADDR + (BEATSIZE+1) * 1
 *** - DMA_BITCOUNT_VALUE SPI_DATA_SIZE transfer before waking up the core
 ***/
void DMA_RX_descriptor_init(void)
{
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.VALID = 1;
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.EVOSEL = DMAC_BTCTRL_EVOSEL_DISABLE_Val;
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.BLOCKACT = DMAC_BTCTRL_BLOCKACT_INT_Val;    
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.BEATSIZE = DMAC_BTCTRL_BEATSIZE_BYTE_Val;    
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.SRCINC = 0;                        
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.DSTINC = 1;
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.STEPSEL = DMAC_BTCTRL_STEPSEL_DST_Val;
 descriptor_SPI[SPI_RX_CHANNEL].BTCTRL.bit.STEPSIZE = DMAC_BTCTRL_STEPSIZE_X1_Val;
 descriptor_SPI[SPI_RX_CHANNEL].BTCNT.reg = SPI_DATA_SIZE+1;
 
 /*** Set source address and destination address 
  *** source address is the SPI Data register (SERCOM0->SPI.DATA)
  *** destination address in the RAM is the rx buffer (spi_rx_buffer[SPI_DATA_SIZE])
  ***/
 descriptor_SPI[SPI_RX_CHANNEL].SRCADDR.reg = (uint32_t) (&(SERCOM0->SPI.DATA));
 descriptor_SPI[SPI_RX_CHANNEL].DSTADDR.reg = (uint32_t) (&(spi_rx_buffer[SPI_DATA_SIZE]));
 
 /*** Set next transfer descriptor_SPI_TX address ***/
 descriptor_SPI[SPI_RX_CHANNEL].DESCADDR.reg = (uint32_t) (&descriptor_SPI[SPI_TX_CHANNEL]);
}
 
/*************************************************/
/*** DMA RX Descriptor Initialization step by step
 *** - Validate the Descriptor
 *** - No Event out generated by the DMA
 *** - Channel will be disabled if it is the last block 
 ***   transfer in the transaction and block interrupt
 *** - 8-bit bus transfer (BYTE)
 *** - Enable Increment from the  the source address (SRCINC=1)
 *** - No increment from the destination address (DSTINC=0)
 *** - Step size settings apply to the Source address
 *** - Step Size => Next ADDR = ADDR + (BEATSIZE+1) * 1
 *** - DMA_BITCOUNT_VALUE SPI_DATA_SIZE transfer before waking up the core
 ***/
void DMA_TX_descriptor_init(void)
{
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.VALID = 1;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.EVOSEL = DMAC_BTCTRL_EVOSEL_DISABLE_Val;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.BLOCKACT = DMAC_BTCTRL_BLOCKACT_INT_Val;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.BEATSIZE = DMAC_BTCTRL_BEATSIZE_BYTE_Val;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.SRCINC = 1;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.DSTINC = 0;
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.STEPSEL = DMAC_BTCTRL_STEPSEL_SRC_Val;    
 descriptor_SPI[SPI_TX_CHANNEL].BTCTRL.bit.STEPSIZE = DMAC_BTCTRL_STEPSIZE_X1_Val;
// descriptor_SPI[SPI_TX_CHANNEL].BTCNT.reg = SPI_DATA_SIZE+1;
 
 /*** Set source address and destination address 
  *** Source address in the RAM is the spi Output buffer (spi_output_data[SPI_DATA_SIZE])
  *** Destination address is the SPI Data register (SERCOM0->SPI.DATA)
  ***/
 descriptor_SPI[SPI_TX_CHANNEL].SRCADDR.reg = (uint32_t) (&(spi_output_data[SPI_DATA_SIZE]));
 descriptor_SPI[SPI_TX_CHANNEL].DSTADDR.reg = (uint32_t) (&(SERCOM0->SPI.DATA));
 
 /*** Set next transfer address to NULL ***/
 descriptor_SPI[SPI_TX_CHANNEL].DESCADDR.reg = NULL;
}
 
/*************************************************/
/*** DMA SPI TX and RX Channels initialization ***/
void DMA_init_SPI(void)
{
 /***
  *** Setup descriptor base address 
  *** and write back section base address 
  ***/
 DMAC->BASEADDR.reg = (uint32_t)descriptor_SPI;
 DMAC->WRBADDR.reg = (uint32_t)write_back_descriptor_SPI;
 
 /***
  *** Disable DMAC 
  *** Enable all priority level at the same time 
  *** Round Robin Arbitrer Scheme selected
  ***/
 DMAC->CTRL.reg &= ~(DMAC_CTRL_DMAENABLE);
 DMAC->CTRL.reg |= DMAC_CTRL_LVLEN(0xf);    
 DMAC->PRICTRL0.bit.RRLVLEN0 = 1;
 
 /***
  *** Configure The TX SPI DMA Channel:
  *** DMAC Channel 1 is used
  *** Disable the channel before configuring it
  ***/
 DMAC->CHID.reg = DMAC_CHID_ID(SPI_TX_CHANNEL);
 DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;
 
 /***
  *** Configure TX SPI DMA Channel: (continue...)
  *** DMA is used in STANDBY, therefore needs RUNSTDBY 
  *** Channel Priority Level = 0
  *** SERCOM0 (SPI) is used as source to trigger the DMA TX
  *** The trigger action is done for every DMA BEAT
  ***/
 DMAC->CHCTRLA.reg = DMAC_CHCTRLA_RUNSTDBY;
 DMAC->CHCTRLB.reg = (DMAC_CHCTRLB_LVL(0x0)|    
                      DMAC_CHCTRLB_TRIGSRC(SERCOM0_DMAC_ID_TX)| 
                      DMAC_CHCTRLB_TRIGACT_BEAT);
 
 /*** Enabling DMA channel TX ***/
 DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
 while(DMAC->CHSTATUS.bit.BUSY);  
 
 /***
  *** Configure RX SPI DMA Channel:
  *** DMAC Channel 0 is used
  *** Disable the channel before configuring it
  ***/
 DMAC->CHID.reg = DMAC_CHID_ID(SPI_RX_CHANNEL);    
 DMAC->CHCTRLA.reg &= ~DMAC_CHCTRLA_ENABLE;    
 
 /***
  *** Configure RX SPI DMA Channel: (continue...)
  *** DMA is used in STANDBY, therefore needs RUNSTDBY 
  *** Channel Priority Level = 0
  *** SERCOM0 (SPI) is used as source to trigger the DMA RX
  *** The trigger action is done for every DMA BEAT
  ***/
  DMAC->CHCTRLA.reg = DMAC_CHCTRLA_RUNSTDBY;
  DMAC->CHCTRLB.reg = ( DMAC_CHCTRLB_LVL(0x0)|    
                        DMAC_CHCTRLB_TRIGSRC(SERCOM0_DMAC_ID_RX)|
                        DMAC_CHCTRLB_TRIGACT_BEAT);
 
 /*** enabling DMA interrupt on RX transfer completed ***/
 DMAC->CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;        
 
 /*** Enabling DMA channel RX ***/
 DMAC->CHCTRLA.reg |= DMAC_CHCTRLA_ENABLE;
 while(DMAC->CHSTATUS.bit.BUSY);  
 
 /*** Enabling interrupt at core side ***/
 NVIC_EnableIRQ(DMAC_0_IRQn);
 
 /*** Finally Enable the DMA 
  *** (Enable Protected)
  ***/
 DMAC->CTRL.reg |= (DMAC_CTRL_DMAENABLE) ;    
}
 
/***********************************************************/
/*** Function That initializes the SERCOM to work in SPI ***/
void Spi_Init(void) 
{
 /*** port & clock to be configured and enable first 
  *** Considering that OSC16M is enabled by defaut running 
  *** @4MHz, the following lines configure the Generic Clock
  *** Generator 4 (GCLK4) to provide a clock to the SERCOM0.
 ***/
 GCLK->GENCTRL[4].reg = GCLK_GENCTRL_DIV(PWM_HIGH_GCLK_RATIO)|GCLK_GENCTRL_SRC_OSC16M|GCLK_GENCTRL_GENEN;
 while((GCLK->SYNCBUSY.reg & GCLK_SYNCBUSY_GENCTRL4));
 GCLK->PCHCTRL[GCLK_SERCOM0_CORE].reg = (GCLK_PCHCTRL_CHEN|GCLK_PCHCTRL_GEN_GCLK4);
 
 /*** configure SERCOM0 I/Os (SPI) 
  *** PA04: SERCOM0 PAD[0]: SPI MISO
  *** PA14: SERCOM0 PAD[2]: SPI MOSI
  *** PA05: SERCOM0 PAD[1]: SPI SS
  *** PA15: SERCOM0 PAD[3]: SPI SCK
  ***/
 PORT->Group[0].WRCONFIG.reg = (uint32_t)(PORT_WRCONFIG_WRPINCFG|
                                          PORT_WRCONFIG_WRPMUX|
                                          PORT_WRCONFIG_PINMASK(1<<5|1<<4|1<<15|1<<14)|
                                          PORT_WRCONFIG_PMUXEN|PORT_WRCONFIG_PMUX(3));    
 
 /***    
  *** Reset the SPI Mode prior any configuration this will also disable the SERCOM
  *** Set SPI mode to master
  *** Set SPI transfer mode to CPOL=0,CPHA=0
  *** Set SPI frame format (std. SPI Frame)
  *** Set Data out pinout
  *** Set Data In pinout
  *** Set Data Order to MSB first
  ***/
 SERCOM0->SPI.CTRLA.bit.SWRST = 1;
 while(SERCOM0->SPI.SYNCBUSY.bit.SWRST);
 SERCOM0->SPI.CTRLA.reg = ( SERCOM_SPI_CTRLA_MODE(0x03)|
                            SERCOM_SPI_CTRLA_FORM(0x00)|
                            SERCOM_SPI_CTRLA_DIPO(0x00)|
                            SERCOM_SPI_CTRLA_DOPO(0x01));
 
 /*** Set SPI Character size to 8-bit
  *** Enable the receiver
  *** Set the baud rate to 0 that ensures to have Fbaud = 2MHZ
  *** knowing that Fbaud is Fref/2 (OSC16M running @ 4MHz)
  ***/
 SERCOM0->SPI.CTRLB.reg = ( SERCOM_SPI_CTRLB_CHSIZE(0x00)|
                            SERCOM_SPI_CTRLB_RXEN);
 SERCOM0->SPI.BAUD.reg = 0;
 
 /*** Disable SPI ***/
 SERCOM0->SPI.CTRLA.bit.ENABLE = 0 ;
 while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE);
 
}
 
/*************************************************/
/*** Start Transfer ***/
void Spi_TxData(uint8_t data)
{
 /*** REconfigure SERCOM0 I/Os (SPI)
  *** PA04: SERCOM0 PAD[0]: SPI MISO
  *** PA14: SERCOM0 PAD[2]: SPI MOSI
  *** PA05: SERCOM0 PAD[1]: SPI SS
  *** PA15: SERCOM0 PAD[3]: SPI SCK
  ***/
 PORT->Group[0].WRCONFIG.reg = (uint32_t)(PORT_WRCONFIG_WRPINCFG|
                                          PORT_WRCONFIG_WRPMUX|
                                          PORT_WRCONFIG_PINMASK(1<<5|1<<4|1<<15|1<<14)|
                                          PORT_WRCONFIG_PMUXEN|PORT_WRCONFIG_PMUX(3));    
 
 /*** 
  *** ReInitialize the DMA descriptors and Cionfigure
  *** and enable the channel
  *** DMAC Channel Description 
  ***/
 DMA_RX_descriptor_init();
 DMA_TX_descriptor_init();
 DMA_init_SPI();
 
 /*** Enable The SERCOM SPI to start the transfer ***/
 SERCOM0->SPI.CTRLA.bit.ENABLE = 1 ;
 while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE);
}
 
/*************************************************/
/***
 *** Shut down the SPI, if requred; called after all bytes sent
 *** Port are also put in tristate to prevent over consumption (if required)
 ***/
void Spi_Done(void) 
{
    SERCOM0->SPI.CTRLA.bit.ENABLE = 0 ;
    while(SERCOM0->SPI.SYNCBUSY.bit.ENABLE);       
 
 /*** disable DMAC ***/
 DMAC->CTRL.reg    &= ~(DMAC_CTRL_DMAENABLE) ;
 
 /*** port & clock to be disabled ***/
 PORT->Group[0].WRCONFIG.reg = (uint32_t)(PORT_WRCONFIG_WRPINCFG|
                                          PORT_WRCONFIG_WRPMUX|
                                          PORT_WRCONFIG_PINMASK(1<<5|1<<4|1<<15|1<<14)|
                                          PORT_WRCONFIG_PMUX(0));    
}
