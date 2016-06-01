// -----------------------------------------------------------------------------
// USB Module
// ----------
// 
// General      :   The module initializes the USB devices and analyzes them; it
//                  provides an interface to perform operations on UHCI devices.
//
// Input        :   None
//
// Process      :   initializes the USB devices and analyzes them; provides an
//                  interface to perform operations on UHCI devices.
//
// Output       :   None
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------


#include <usb.h>


QH *ctrl_qh;
QH *bulk_in_qh;
QH *bulk_out_qh;
void *frame_list;
void *data_page;
TD *last_td;
TD*next_td;
int ehci_disabled;
int setup;
char next_tag;
uint16_t next_addr;


void init_uhci(void) {
    frame_list = palloc();
    data_page = palloc();

    reset_qh();

    next_addr = 1;

    next_tag = FIRST_TAG;
    setup = 1;
    ehci_disabled = 0;
    check_all_buses();
    ehci_disabled = 1;
    check_all_buses();
    setup = 0;
}

void config_usb(uint8_t bus, uint8_t device, uint8_t function) {
    uint8_t prog_if;

    if (setup) {
        prog_if = get_prog_if(bus, device, function);
        switch (prog_if) {
            case USB_UHCI:
                if (ehci_disabled)
                    config_uhci(bus, device, function);
                break;
            case USB2:
                if (!ehci_disabled)
                    config_ehci(bus, device, function);
                break;
        }
    }
}

void config_ehci(uint8_t bus, uint8_t device, uint8_t function) {
    uint32_t usbbase, eecp, hcbiossem, opregs, capid;
    uint8_t reg;

    pci_cfg_write_w(bus, device, function, 0x04, 0x06);

    usbbase = 0;
    for (reg = 0x10; reg <= 0x24 && !usbbase; reg += 4) {
        usbbase = pci_cfg_read(bus, device, function, reg);
    }

    if (usbbase) {
        eecp = (*((uint32_t *) (usbbase + 0x08)) & 0xFF00) >> 8;
        if (eecp >= 0x40) {
            hcbiossem = pci_cfg_read(bus, device, function, eecp) & 0x00010000;
            if (hcbiossem) {
                pci_cfg_write_b(bus, device, function, eecp + 0x03, 0x01);
                wait_ticks(1024);
                capid = pci_cfg_read(bus, device, function, eecp) & 0xFF;
                if (capid == 0x01) {
                    pci_cfg_write_w(bus, device, function, eecp + 0x04, 0x0000);
                }
            }
        }
        opregs = usbbase + (*((uint32_t *) usbbase) & 0xFF);
        *((uint32_t *) (opregs + 0x40)) = 0;
    }
}

void config_uhci(uint8_t bus, uint8_t device, uint8_t function) {
    UHCIDevice *dev;
    DevDesc *dev_desc;
    ConfigDesc *config_desc;
    InterDesc *inter_desc;
    EndpDesc *endp_desc;
    void *next_desc;
    uint16_t sctemp;
    int i, j, b_inter, b_endp_in, b_endp_out;

    // Get PCI data about device
    dev = (UHCIDevice *) malloc(sizeof (UHCIDevice));
    dev->bus = bus;
    dev->device = device;
    dev->function = function;
    dev->vendor_id = get_vendor_id(bus, device, function);
    dev->device_id = get_device_id(bus, device, function);
    dev->maxp = 8;
    dev->addr = 0;
    dev->base_port = pci_cfg_read(bus, device, function, USBBASE_PCI_REG) & 0xFFE0;
    dev->tag = next_tag++;

    // Disable the device's Interrupts
    OUTW(dev->base_port + USBINTR, 0);
    // Get the port status
    INW(sctemp, dev->base_port + PORTSC1);
    if (!(sctemp & PORTSC_CONNECTED)) {
        return;
    }

    if ((sctemp & PORTSC_CONNECTED) && !(sctemp & PORTSC_ENABLED)) {
        free((void *) dev);
        return;
    }

    // Disable the device's Interrupts
    OUTW(dev->base_port + USBINTR, 0);
    // Disable the device's Command register
    OUTW(dev->base_port + USBCMD, 0);
    // Disable the device's Status register
    OUTW(dev->base_port + USBSTS, USBSTS_MASK);
    // Wait at least 1ms (1 frame)
    wait_ticks(2);
    // Disable the legacy BIOS support
    pci_cfg_write_l(bus, device, function, UHCI_PCI_LEGACY_SUPPORT, UHCI_PCI_LEGACY_SUPPORT_STATUS);
    // reset the device
    reset_uhci(dev);

    dev_desc = get_dev_desc(dev, 0);
    if (!dev_desc->num_config) {
        return;
    }
    dev->config = 1;
    dev->maxp = dev_desc->maxp;

    if (set_addr(dev, next_addr++) == USB_TD_ERROR) {
        return;
    }

    config_desc = get_config_desc(dev, 0);
    if (!config_desc) {
        return;
    }
    next_desc = ((void *) config_desc) + sizeof (ConfigDesc);
    b_inter = b_endp_in = b_endp_out = 0;
    for (i = 0; i < config_desc->num_inter; i++) {
        if (config_desc->total_len > next_desc - ((void *) config_desc)) {
            inter_desc = next_desc;
            next_desc += sizeof (InterDesc);
            if (inter_desc->class == CLASS_MASS_STORAGE &&
                    inter_desc->subclass == SUBCLASS_SCSI &&
                    inter_desc->proto == PROTO_BBB) {
                dev->interface = inter_desc->inter_num;
                b_inter = 1;
                for (j = 0; j < inter_desc->num_endp; j++) {
                    if (config_desc->total_len > next_desc - ((void *) config_desc)) {
                        endp_desc = next_desc;
                        next_desc += sizeof (EndpDesc);
                        if (endp_desc->addr & (1 << 7)) {
                            dev->in_endp = endp_desc->addr & 0xF;
                            dev->in_maxp = endp_desc->maxp;
                            b_endp_in = 1;
                        } else {
                            dev->out_endp = endp_desc->addr & 0xF;
                            dev->out_maxp = endp_desc->maxp;
                            b_endp_out = 1;
                        }
                    }
                }
            } else {
                next_desc += inter_desc->num_endp * sizeof (EndpDesc);
            }
        }
    }

    if (!b_inter || !b_endp_in || !b_endp_out) {
        return;
    }

    if (set_config(dev, dev->config) == USB_TD_ERROR) {
        return;
    }

    if (set_inter(dev, dev->interface) == USB_TD_ERROR) {
        return;
    }

    if (init_bbb(dev) == USB_TD_ERROR) {
        return;
    }

    init_fs(dev);

    free(dev_desc);
    free(config_desc);
}

TD *add_td(UHCIDevice *dev,
        uint32_t pid,
        uint32_t end_point,
        uint32_t max_len,
        uint32_t buff_ptr,
        uint32_t data_toggle,
        uint32_t short_packet,
        QH *qh) {
    TD *td;

    td = next_td;
    next_td++;
    if (last_td)
        last_td->link_pointer = ((uint32_t) td);
    else
        qh->element_pointer = ((uint32_t) td);
    last_td = td;

    td->link_pointer = LP_TERMINATE;

    td->act_len = 0x7FF;
    td->reserved1 = 0;
    td->status = TD_STATUS_ACTIVE;
    td->ioc = 0;
    td->ios = 0;
    td->low_speed = 0;
    td->err_limit = 0;
    td->spd = short_packet;
    td->reserved2 = 0;

    td->packet_id = pid;
    td->device_addr = dev->addr;
    td->end_point = end_point;
    td->data_toggle = data_toggle;
    td->reserved3 = 0;
    td->max_len = (max_len - 1) & 0x7FF;

    td->buff_ptr = buff_ptr;

    return td;
}

int run_qh(UHCIDevice *dev) {
    uint16_t temp;
    TD *td;
    char *buff;

    OUTL(dev->base_port + FRBASEADD, (uint32_t) frame_list);
    INW(temp, dev->base_port + FRNUM);
    // Zero FRNUM
    temp &= 0xF800;
    OUTW(dev->base_port + FRNUM, temp);
    INW(temp, dev->base_port + USBCMD);
    temp |= USBCMD_RUN;

    wait_ticks(2);

    OUTW(dev->base_port + USBCMD, temp);

    uint32_t c;
    for (td = (TD *) (data_page + 3 * sizeof (TD)); td <= last_td && last_td; td++) {
        c = 256;
        while (td->status & TD_STATUS_ACTIVE) {
            wait_ticks(8);
            c--;
            if (!c)
                break;
        }
        if (td->status) {
            CLEAR_INTS();
            buff = malloc(20);
            puts("\nUSB ERROR: STATUS = ");
            puts(uitoa(td->status, buff, BASE2));
            HALT();
        }
    }

    INW(temp, dev->base_port + USBCMD);
    temp &= ~USBCMD_RUN;

    wait_ticks(2);
    return 0;
}

void reset_qh(void) {
    uint32_t *temp_ptr;
    int i;

    ctrl_qh = (QH *) (data_page);
    bulk_out_qh = (QH *) (data_page + 1 * sizeof (TD));
    bulk_in_qh = bulk_out_qh;
    last_td = 0;
    next_td = (TD *) (data_page + 2 * sizeof (TD));

    ctrl_qh->link_pointer = ((uint32_t) bulk_out_qh) | LP_QH_SELECT;
    ctrl_qh->element_pointer = LP_TERMINATE;
    bulk_out_qh->link_pointer = LP_TERMINATE;
    bulk_out_qh->element_pointer = LP_TERMINATE;

    temp_ptr = (uint32_t *) frame_list;
    for (i = 0; i < 1024; i++)
        *temp_ptr++ = ((uint32_t) ctrl_qh) | LP_QH_SELECT;
}

void reset_uhci(UHCIDevice *dev) {
    uint16_t temp;

    INW(temp, dev->base_port + USBCMD);
    OUTW(dev->base_port + USBCMD, temp | 1);
    wait_ticks(256);
    OUTW(dev->base_port + PORTSC1, (1 << 9));
    wait_ticks(256);
    OUTW(dev->base_port + PORTSC1, 0);
    wait_ticks(128);
    OUTW(dev->base_port + PORTSC1, PORTSC_ENABLED_CHANGED | PORTSC_ENABLED | PORTSC_CONNECTED_CHANGED);
    wait_ticks(256);
    OUTW(dev->base_port + USBCMD, USBCMD_GRESET);
    wait_ticks(256);
    OUTW(dev->base_port + USBCMD, USBCMD_HCRESET);
    wait_ticks(256);
    OUTW(dev->base_port + USBINTR, 0);
    OUTW(dev->base_port + PORTSC1, PORTSC_ENABLED);
    wait_ticks(256);
}

TD *setup_req(UHCIDevice *dev,
        uint32_t end_point,
        uint32_t data_toggle,
        uint8_t req_type,
        uint8_t req,
        uint16_t value,
        uint16_t index,
        uint16_t length) {
    SETUPReq *setreq;

    setreq = (SETUPReq *) malloc(sizeof (SETUPReq));
    setreq->req_type = req_type;
    setreq->req = req;
    setreq->value = value;
    setreq->index = index;
    setreq->length = length;
    return add_td(dev, SETUP, end_point, sizeof (SETUPReq), (uint32_t) setreq, data_toggle, 0, ctrl_qh);
}

TD *input_req(UHCIDevice *dev, uint32_t end_point, uint32_t data_toggle, uint32_t length, void *data) {
    return add_td(dev, IN, end_point, length, (uint32_t) data, data_toggle, 0, ctrl_qh);
}

void *get_desc(UHCIDevice *dev, uint8_t desc_type, uint8_t desc_index, uint16_t desc_length) {
    void *data;

    setup_req(
            dev,
            0,
            DATA0,
            DEVICE_TO_HOST | TYPE_STANDARD | RECP_DEVICE,
            SETUP_GET_DESC,
            (((uint16_t) desc_type) << 8) | ((uint16_t) desc_index),
            0,
            desc_length
            );
    data = malloc(desc_length);
    input_req(dev, 0, DATA1, desc_length, data);
    add_td(dev, OUT, 0, 0, 0, DATA1, 0, ctrl_qh);
    if (run_qh(dev))
        return 0;
    reset_qh();
    return data;
}

int set_addr(UHCIDevice *dev, uint16_t dev_addr) {
    setup_req(dev,
            0,
            DATA0,
            HOST_TO_DEVICE | TYPE_STANDARD | RECP_DEVICE,
            SETUP_SET_ADDRESS,
            dev_addr,
            0,
            0);
    add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);

    if (run_qh(dev))
        return USB_TD_ERROR;
    reset_qh();

    dev->addr = dev_addr;

    return 0;
}

int set_config(UHCIDevice *dev, uint16_t config) {
    setup_req(dev,
            0,
            DATA0,
            HOST_TO_DEVICE | TYPE_STANDARD | RECP_DEVICE,
            SETUP_SET_CONFIG,
            config,
            0,
            0);
    add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
    if (run_qh(dev))
        return USB_TD_ERROR;
    reset_qh();

    return 0;
}

int set_inter(UHCIDevice *dev, uint16_t inter) {
    setup_req(dev,
            0,
            DATA0,
            HOST_TO_DEVICE | TYPE_STANDARD | RECP_INTERFACE,
            SETUP_SET_INTERFACE,
            inter,
            0,
            0);
    add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
    if (run_qh(dev))
        return USB_TD_ERROR;

    reset_qh();
    return 0;
}

DevDesc *get_dev_desc(UHCIDevice *dev, uint8_t dev_index) {
    return (DevDesc *) get_desc(dev, DESC_TYPE_DEVICE, dev_index, sizeof (DevDesc));
}

ConfigDesc *get_config_desc(UHCIDevice *dev, uint8_t config_index) {
    return (ConfigDesc *) get_desc(dev, DESC_TYPE_CONFIG, config_index, 64);
}