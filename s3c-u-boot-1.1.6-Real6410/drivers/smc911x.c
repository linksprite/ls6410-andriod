/*
 * SMSC LAN9[12]1[567] Network driver
 *
 * (c) 2007 Pengutronix, Sascha Hauer <[EMAIL PROTECTED]>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>

#ifdef CONFIG_DRIVER_SMC911X

#include <command.h>
#include <net.h>
#include <miiphy.h>

#include "smc911x.h"

struct chip_id {
       u16 id;
       char *name;
};

static const struct chip_id chip_ids[] =  {
       { CHIP_9115, "LAN9115" },
       { CHIP_9116, "LAN9116" },
       { CHIP_9117, "LAN9117" },
       { CHIP_9118, "LAN9118" },
       { CHIP_9215, "LAN9215" },
       { CHIP_9216, "LAN9216" },
       { CHIP_9217, "LAN9217" },
       { CHIP_9218, "LAN9218" },
       { 0, NULL },
};

#define DRIVERNAME "smc911x"

u32 smc911x_get_mac_csr(u8 reg)
{
       while (MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY);
       MAC_CSR_CMD = MAC_CSR_CMD_CSR_BUSY | MAC_CSR_CMD_R_NOT_W | reg;
       while (MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY);

       return MAC_CSR_DATA;
}

void smc911x_set_mac_csr(u8 reg, u32 data)
{
       while (MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY);
       MAC_CSR_DATA = data;
       MAC_CSR_CMD = MAC_CSR_CMD_CSR_BUSY | reg;
       while (MAC_CSR_CMD & MAC_CSR_CMD_CSR_BUSY);
}

static int smx911x_handle_mac_address(bd_t *bd)
{
       unsigned long addrh, addrl;
       unsigned char *m = bd->bi_enetaddr;

       /* if the environment has a valid mac address then use it */
       if ((m[0] | m[1] | m[2] | m[3] | m[4] | m[5])) {
               addrl = m[0] | m[1] << 8 | m[2] << 16 | m[3] << 24;
               addrh = m[4] | m[5] << 8;
               smc911x_set_mac_csr(ADDRH, addrh);
               smc911x_set_mac_csr(ADDRL, addrl);
       } else {
               /* if not, try to get one from the eeprom */
               addrh = smc911x_get_mac_csr(ADDRH);
               addrl = smc911x_get_mac_csr(ADDRL);

               m[0] = (addrl       ) & 0xff;
               m[1] = (addrl >>  8 ) & 0xff;
               m[2] = (addrl >> 16 ) & 0xff;
               m[3] = (addrl >> 24 ) & 0xff;
               m[4] = (addrh       ) & 0xff;
               m[5] = (addrh >>  8 ) & 0xff;

               /* we get 0xff when there is no eeprom connected */
               if ((m[0] & m[1] & m[2] & m[3] & m[4] & m[5]) == 0xff) {
                       printf(DRIVERNAME ": no valid mac address in environment "
                               "and no eeprom found\n");
                       return -1;
               }
       }

       printf(DRIVERNAME ": MAC %02x:%02x:%02x:%02x:%02x:%02x\n",
               m[0], m[1], m[2], m[3], m[4], m[5]);

       return 0;
}

/* TODO 2008.05.09*/
#if 0 
static int smc911x_miiphy_read(u8 phy, u8 reg, u16 *val)
{
       while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY);

       smc911x_set_mac_csr( MII_ACC, phy << 11 | reg << 6 | MII_ACC_MII_BUSY);

       while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY);

       *val = smc911x_get_mac_csr(MII_DATA);

       return 0;
}
#endif

static int smc911x_miiphy_write(u8 phy, u8 reg, u16  val)
{
       while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY);

       smc911x_set_mac_csr(MII_DATA, val);
       smc911x_set_mac_csr(MII_ACC,
               phy << 11 | reg << 6 | MII_ACC_MII_BUSY | MII_ACC_MII_WRITE);

       while (smc911x_get_mac_csr(MII_ACC) & MII_ACC_MII_BUSY);
       return 0;
}

static int smc911x_phy_reset(void)
{
       u32 reg;

       reg = PMT_CTRL;
       reg &= ~0xfffff030;
       reg |= PMT_CTRL_PHY_RST;
       PMT_CTRL = reg;

/* TODO 2008.05.09*/
       //mdelay(100);
       mdelay(10);

       return 0;
}

static void smc911x_phy_configure(void)
{
/* TODO 2008.05.09*/
#if 0 
       int timeout;
       u16 status;
#endif

       smc911x_phy_reset();

       smc911x_miiphy_write(1, PHY_BMCR, PHY_BMCR_RESET);
       mdelay(1);
       smc911x_miiphy_write(1, PHY_ANAR, 0x01e1);
       smc911x_miiphy_write(1, PHY_BMCR, PHY_BMCR_AUTON | PHY_BMCR_RST_NEG);

/* TODO 2008.05.09*/
#if 0 
       timeout = 5000;
       do {
               mdelay(1);
               if ((timeout--) == 0)
                       goto err_out;

               if (smc911x_miiphy_read(1, PHY_BMSR, &status) != 0)
                       goto err_out;
       } while (!(status & PHY_BMSR_LS));

#endif
       printf(DRIVERNAME ": phy initialized\n");

       return;

/* TODO 2008.05.09*/
#if 0 
err_out:
       printf(DRIVERNAME ": autonegotiation timed out\n");
#endif
}

static void smc911x_reset(void)
{
       int timeout;

       /* Take out of PM setting first */
       if (PMT_CTRL & PMT_CTRL_READY) {
               /* Write to the bytetest will take out of powerdown */
               BYTE_TEST = 0x0;

               timeout = 10;

               while ( timeout-- && !(PMT_CTRL & PMT_CTRL_READY))
                       udelay(10);
               if (!timeout) {
                       printf(DRIVERNAME
                               ": timeout waiting for PM restore\n");
                       return;
               }
       }

       /* Disable interrupts */
       INT_EN = 0;

       HW_CFG = HW_CFG_SRST;

       timeout = 1000;
       while (timeout-- && E2P_CMD & E2P_CMD_EPC_BUSY)
               udelay(10);

       if(!timeout) {
               printf(DRIVERNAME ": reset timeout\n");
               return;
       }

       /* Reset the FIFO level and flow control settings */
       smc911x_set_mac_csr(FLOW, FLOW_FCPT | FLOW_FCEN);
       AFC_CFG = 0x0050287F;

       /* Set to LED outputs */
       GPIO_CFG = 0x70070000;
}

static void smc911x_enable(void)
{
       /* Enable TX */
       HW_CFG = 8 << 16 | HW_CFG_SF;

       GPT_CFG = GPT_CFG_TIMER_EN | 10000;

       TX_CFG = TX_CFG_TX_ON;

       /* no padding to start of packets */
       RX_CFG = 0;

       smc911x_set_mac_csr(MAC_CR, MAC_CR_TXEN | MAC_CR_RXEN | MAC_CR_HBDIS);

}

int eth_init(bd_t *bd)
{
       unsigned long val, i;

       printf(DRIVERNAME ": initializing\n");

       val = BYTE_TEST;
       if(val != 0x87654321) {
               printf(DRIVERNAME ": Invalid chip endian 0x08%x\n", val);
               goto err_out;
       }

       val = ID_REV >> 16;
       for(i = 0; chip_ids[i].id != 0; i++) {
               if (chip_ids[i].id == val) break;
       }
       if (!chip_ids[i].id) {
               printf(DRIVERNAME ": Unknown chip ID %04x\n", val);
               goto err_out;
       }

       printf(DRIVERNAME ": detected %s controller\n", chip_ids[i].name);

       smc911x_reset();

       /* Configure the PHY, initialize the link state */
       smc911x_phy_configure();

       if (smx911x_handle_mac_address(bd))
               goto err_out;

       /* Turn on Tx + Rx */
       smc911x_enable();

       return 0;

err_out:
       return -1;
}

int eth_send(volatile void *packet, int length)
{
       u32 *data = (u32*)packet;
       u32 tmplen;
       u32 status;

       TX_DATA_FIFO = TX_CMD_A_INT_FIRST_SEG | TX_CMD_A_INT_LAST_SEG | length;
       TX_DATA_FIFO = length;

       tmplen = (length + 3) / 4;

       while(tmplen--)
               TX_DATA_FIFO = *data++;

       /* wait for transmission */
       while (!((TX_FIFO_INF & TX_FIFO_INF_TSUSED) >> 16));

       /* get status. Ignore 'no carrier' error, it has no meaning for
        * full duplex operation
        */
       status = TX_STATUS_FIFO & (TX_STS_LOC | TX_STS_LATE_COLL |
               TX_STS_MANY_COLL | TX_STS_MANY_DEFER | TX_STS_UNDERRUN);

       if(!status)
               return 0;

       printf(DRIVERNAME ": failed to send packet: %s%s%s%s%s\n",
               status & TX_STS_LOC ? "TX_STS_LOC " : "",
               status & TX_STS_LATE_COLL ? "TX_STS_LATE_COLL " : "",
               status & TX_STS_MANY_COLL ? "TX_STS_MANY_COLL " : "",
               status & TX_STS_MANY_DEFER ? "TX_STS_MANY_DEFER " : "",
               status & TX_STS_UNDERRUN ? "TX_STS_UNDERRUN" : "");

       return -1;
}

void eth_halt(void)
{
       smc911x_reset();
}

int eth_rx(void)
{
       u32 *data = (u32 *)NetRxPackets[0];
       u32 pktlen, tmplen;
       u32 status;

       if((RX_FIFO_INF & RX_FIFO_INF_RXSUSED) >> 16) {
               status = RX_STATUS_FIFO;
               pktlen = (status & RX_STS_PKT_LEN) >> 16;

               RX_CFG = 0;

               tmplen = (pktlen + 2+ 3) / 4;
               while(tmplen--)
                       *data++ = RX_DATA_FIFO;

               if(status & RX_STS_ES)
                       printf(DRIVERNAME
                               ": dropped bad packet. Status: 0x%08x\n",
                               status);
               else
                       NetReceive(NetRxPackets[0], pktlen);
       }

       return 0;
}

#endif                         /* CONFIG_DRIVER_SMC911X */
