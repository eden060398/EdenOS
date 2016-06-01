// -----------------------------------------------------------------------------
// Bulk-Only Trasport (BBB) Module
// -------------------------------
// 
// General  :   The module initializes a BBB-type USB device and provides an
//              interface to perform operations on it.
//
// Input    :   None.
//
// Process  :   Reset the BBB interface of the device and perform the requested
//              operations on it.
//
// Output   :   None.
//
// -----------------------------------------------------------------------------
// Programmer   :   Eden Frenkel
// -----------------------------------------------------------------------------


#include <bbb.h>

static uint32_t tag;
static uint32_t data_in;
static uint32_t data_out;


// -----------------------------------------------------------------------------
// init_bbb
// --------
// 
// General  :   The function initializes the BBB interface on the USB device.
//
// Parameters   :
//              dev -   A pointer to the USB device descriptor (In)
//
// Return Value	:   0 if successful, otherwise an error specifier (int)
//
// -----------------------------------------------------------------------------

int init_bbb(UHCIDevice *dev) {
    // Initialize bulk DATA TOGGLE.
    data_in = 1;
    data_out = 1;
    // Initialize the Transport Tag.
    tag = 0;

    // Reset the BBB interface.
    if (bbb_reset(dev) == USB_TD_ERROR)
        return USB_TD_ERROR;

    return 0;
}

// -----------------------------------------------------------------------------
// bbb_reset
// ---------
// 
// General  :   The function resets the BBB interface on the USB
//              device.
//
// Parameters   :
//              dev -   A pointer to the USB device descriptor (In)
//
// Return Value	:   0 if successful, otherwise an error specifier (int)
//
// -----------------------------------------------------------------------------

int bbb_reset(UHCIDevice *dev) {
    // Create a SETUP request for BBB RESET.
    setup_req(dev,
            0,
            DATA0,
            HOST_TO_DEVICE | TYPE_CLASS | RECP_INTERFACE,
            REQ_MASS_RESET,
            0,
            dev->interface,
            0);
    // Create an empty IN request (needed).
    add_td(dev, IN, 0, 0, 0, DATA1, 0, ctrl_qh);
    // Run the requests.
    if (run_qh(dev))
        return USB_TD_ERROR;
    // Reset the interface.
    reset_qh();

    return 0;
}

// -----------------------------------------------------------------------------
// read_bbb
// --------
// 
// General      :   The function reads data from the USB device.
//
// Parameters	:
//              dev	-   A pointer to the USB device descriptor (In)
//              block   -   The LBA of the first block to read (In)
//              count   -   The amount of blocks to read (In)
//              ptr     -   A pointer to the address in memory to write the data
//                          to (Out)
//
// Return Value	:   0 if successful, otherwise an error specifier (int)
//
// -----------------------------------------------------------------------------

int read_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr) {
    void *cbw_ptr;
    void *csw_ptr;
    uint32_t data_addr;
    uint32_t i;
    CBW *cbw;
    CSW *csw;
    int result;

    cbw_ptr = malloc(CBW_LEN);
    csw_ptr = malloc(CSW_LEN);

    // Create the Command-Block Wrapper.
    cbw = cbw_ptr;
    cbw->signature = CBW_SIGNATURE;
    cbw->tag = tag++;
    cbw->trans_length = BLOCK_LEN * count;
    cbw->flags = TO_HOST;
    cbw->lun = 0;
    cbw->cmd_length = 12;
    // Create the Command-Block.
    create_read12_packet(cbw_ptr + sizeof (CBW), block, count);

    data_addr = (uint32_t) ptr;

    // Add an OUT packet for the CBW.
    add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);

    // For every block, add an IN packet for the data transfer.
    for (i = 0; i < count * (BLOCK_LEN / dev->in_maxp); i++) {
        add_td(dev, IN, dev->in_endp, dev->in_maxp, data_addr, DATA_IN, 0, bulk_in_qh);
        data_addr += dev->in_maxp;
    }

    // Add an IN packet for the Command-Status Wrapper.
    add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);

    // Run the requests.
    if (run_qh(dev))
        return USB_TD_ERROR;
    // Reset the interface.
    reset_qh();

    csw = csw_ptr;
    // Get the status of the transfer.
    result = csw->status;

    free(cbw_ptr);
    free(csw_ptr);

    // Return the status of the transfer.
    return result;
}

// -----------------------------------------------------------------------------
// write_bbb
// ---------
// 
// General  :   The function writes data to the USB device.
//
// Parameters   :
//              dev     -   A pointer to the USB device descriptor (In)
//              block   -   The LBA of the first block to write
//              count   -   The amount of blocks to write
//              ptr     -   A pointer to the address in memory to read the data
//                          from
//
// Return Value	:   0 if successful, otherwise an error specifier (int)
//
// -----------------------------------------------------------------------------

int write_bbb(UHCIDevice *dev, uint32_t block, uint32_t count, void *ptr) {
    void *cbw_ptr;
    void *csw_ptr;
    uint32_t data_addr;
    uint32_t i;
    CBW *cbw;
    CSW *csw;
    int result;

    cbw_ptr = malloc(CBW_LEN);
    csw_ptr = malloc(CSW_LEN);

    // Create the Command-Block Wrapper.
    cbw = cbw_ptr;
    cbw->signature = CBW_SIGNATURE;
    cbw->tag = tag++;
    cbw->trans_length = BLOCK_LEN * count;
    cbw->flags = TO_DEVICE;
    cbw->lun = 0;
    cbw->cmd_length = 12;
    // Create the Command-Block.
    create_write12_packet(cbw_ptr + sizeof (CBW), block, count);

    data_addr = (uint32_t) ptr;

    // Add an OUT packet for the CBW.
    add_td(dev, OUT, dev->out_endp, CBW_LEN, (uint32_t) cbw_ptr, DATA_OUT, 0, bulk_out_qh);

    // For every block, add an OUT packet for the data transfer.
    for (i = 0; i < count * (BLOCK_LEN / dev->out_maxp); i++) {
        add_td(dev, OUT, dev->out_endp, dev->out_maxp, data_addr, DATA_OUT, 0, bulk_out_qh);
        data_addr += dev->out_maxp;
    }

    // Add an IN packet for the Command-Status Wrapper.
    add_td(dev, IN, dev->in_endp, CSW_LEN, (uint32_t) csw_ptr, DATA_IN, 0, bulk_in_qh);

    // Run the requests.
    if (run_qh(dev))
        return USB_TD_ERROR;
    // Reset the interface.
    reset_qh();

    csw = csw_ptr;
    // Get the status of the transfer.
    result = csw->status;

    free(cbw_ptr);
    free(csw_ptr);

    // Return the status of the transfer.
    return result;
}
