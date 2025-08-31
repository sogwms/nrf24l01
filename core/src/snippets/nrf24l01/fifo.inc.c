
/**
 * @brief Check if there is free TX FIFO.
 *
 * This function checks if there is at least
 * one available tx-fifo for writing a new packet.
 *
 * @return `true` if has free tx-fifo, `false` otherwise.
 */
int nrf24_txfifo_has_space(nrf24_t *nrf24)
{
    uint8_t fifosta;
    read_reg(&nrf24->dep, NRF24_REG_FIFO_STATUS, &fifosta);
    return !(fifosta & REG_FIFO_STATUS_BITMASK_TX_FULL);
}

/**
 * @brief Check if all TX FIFO are empty (no tx-data).
 *
 * @return `true` if empty, `false` otherwise.
 */
int nrf24_txfifo_is_empty(nrf24_t *nrf24)
{
    uint8_t fifosta;
    read_reg(&nrf24->dep, NRF24_REG_FIFO_STATUS, &fifosta);
    return fifosta & REG_FIFO_STATUS_BITMASK_TX_EMPTY;
}

/**
 * @brief Write data to the TX FIFO.
 *
 * This function writes a packet to the TX FIFO. The behavior depends on the current role:
 * 
 * - PRX: Sends an ACK payload for the specified pipe. (pipe is set using `nrf24_txfifo_set_prx_ackpipe()`)
 * 
 * - PTX: Sends a regular TX payload.
 *
 * @note Caller must ensure there is available(/free) TX FIFO
 * @attention The length of the data must <= 32 bytes (maximum payload size).
 *
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @param[in] data  Pointer to the data buffer to send.
 * @param[in] len   Length of the data to send (must not exceed 32 bytes).
 * @return Zero on success, or negative error code on failure.
 */
int nrf24_txfifo_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len)
{
    int ret = 0;
    if (nrf24->role == NRF24_ROLE_PRX) {
        ret = send_cmd_write_ack_payload(&nrf24->dep, nrf24->ack_pipe, data, len);
    }else {
        ret = send_cmd_write_tx_payload(&nrf24->dep, data, len);
    }

    return ret;
}

/**
 * @brief Set the ACK pipe used by `nrf24_txfifo_write()` in PRX mode.
 *
 * This function sets the pipe number for which the ACK payload will be biding.
 *
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @param[in] pipe  Pipe number (0-5) for which the ACK payload is intended.
 */
void nrf24_txfifo_set_prx_ackpipe(nrf24_t *nrf24, uint8_t pipe)
{
    nrf24->ack_pipe = pipe;
}

uint8_t nrf24_txfifo_get_prx_ackpipe(nrf24_t *nrf24)
{
    return nrf24->ack_pipe;
}

/**
 * @brief Write an ACK payload (PRX mode only).
 *
 * This function is used to set an ACK payload (binding to a specific pipe).
 *
 * @note Caller must ensure there is available(/free) TX FIFO
 * @attention The length of the data must <= 32 bytes (maximum payload size).
 * 
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @param[in] data  Pointer to the ACK payload data.
 * @param[in] len   Length of the ACK payload.
 * @param[in] pipe  Pipe number (0-5) for which the ACK is intended.
 * @return Zero on success, or negative error code on failure.
 */
int nrf24_txfifo_prx_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len, uint8_t pipe)
{ 
    return send_cmd_write_ack_payload(&nrf24->dep, pipe, data, len);
}

/**
 * @brief Write a TX payload (PTX mode only).
 *
 * This function is used to send a regular payload in PTX mode.
 *
 * @note Caller must ensure there is available(/free) TX FIFO
 * @attention The length of the data must <= 32 bytes (maximum payload size).
 * 
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @param[in] data  Pointer to the data to send.
 * @param[in] len   Length of the data to send.
 * @return Zero on success, or negative error code on failure.
 */
int nrf24_txfifo_ptx_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len)
{
    return send_cmd_write_tx_payload(&nrf24->dep, data, len);
}

/**
 * @brief Write a TX payload with no-ack set (PTX mode only).
 *
 * This function is used to send a no-ack packet.
 *
 * @note Caller must ensure there is available(/free) TX FIFO
 * @attention The length of the data must <= 32 bytes (maximum payload size).
 * 
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @param[in] data  Pointer to the data to send.
 * @param[in] len   Length of the data to send.
 * @return Zero on success, or negative error code on failure.
 */
int nrf24_txfifo_ptx_write_no_ack(nrf24_t *nrf24, const uint8_t *data, uint8_t len)
{
    return send_cmd_write_tx_payload_no_ack(&nrf24->dep, data, len);
}

/**
 * @brief Flush (clear) the TX FIFO.
 *
 * This function clears all pending packets in the TX FIFO.
 *
 */
void nrf24_txfifo_flush(nrf24_t *nrf24)
{
    send_cmd_flush_tx(&nrf24->dep);
}

/**
 * @brief Check if the RX FIFO has unread data.
 *
 * This function checks if there is at least one packet in the RX FIFO that can be read.
 *
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @return `true` if unread data exists, `false` otherwise.
 */
int nrf24_rxfifo_has_data(nrf24_t *nrf24)
{
    uint8_t fifosta;
    read_reg(&nrf24->dep, NRF24_REG_FIFO_STATUS, &fifosta);
    return !(fifosta & REG_FIFO_STATUS_BITMASK_RX_EMPTY);
}

/**
 * @brief Check if the RX FIFO is full.
 *
 * This function checks if the RX FIFO is full and cannot accept more packets.
 *
 * @param[in] nrf24 Pointer to the NRF24 device structure.
 * @return `true` if RX FIFO is full, `false` otherwise.
 */
int nrf24_rxfifo_is_full(nrf24_t *nrf24)
{
    uint8_t fifosta;
    read_reg(&nrf24->dep, NRF24_REG_FIFO_STATUS, &fifosta);
    return (fifosta & REG_FIFO_STATUS_BITMASK_RX_RXFULL);
}

/**
 * @brief Fetch received data
 * 
 * @note Caller ensures there are data to fetch, if not, the result is unreliable
 * 
 * @param nrf24 pointer to nrf24 instance
 * @param[in] buf pointer to buffer to store data, must be at least 32 bytes long
 * @param[out] data_len pointer to store data length
 * @param[out] pipe pointer to store pipe number
 * @return int return 0 if success
 */
int nrf24_rxfifo_read(nrf24_t *nrf24, uint8_t *buf, uint8_t *data_len, uint8_t *pipe)
{
    uint8_t sta;

    *data_len = send_cmd_read_rx_payload_width(&nrf24->dep);
    if (*data_len == 0) {
        LOG_D("No data in RX FIFO");
        return -1;
    }

    if (pipe != 0) {
        read_reg(&nrf24->dep, NRF24_REG_STATUS, &sta);
        *pipe = byte_get_bits(sta, REG_STATUS_BITMASK_RX_P_NO);
    }

    send_cmd_read_rx_payload(&nrf24->dep, buf, *data_len);
    
    return 0;
}

/**
 * @brief Flush (clear) the RX FIFO.
 *
 * This function clears all pending packets in the RX FIFO.
 *
 */
void nrf24_rxfifo_flush(nrf24_t *nrf24) 
{
    send_cmd_flush_rx(&nrf24->dep);
}
