#include <zephyr/kernel.h>

K_FIFO_DEFINE(my_fifo);
K_MUTEX_DEFINE(my_mutex);

struct data_item_t {
    void *fifo_reserved;   /* 1st word reserved for use by FIFO */
};

struct data_item_t tx_data;

int i = 0, qty_fifo = 0;

void producer_thread(int unused1, int unused2, int unused3)
{
    while (1) {
        if(qty_fifo < 11){
			if (k_mutex_lock(&my_mutex, K_MSEC(100)) == 0){
				/* create data item to send */
				tx_data = i;

				/* send data to consumers */
				k_fifo_put(&my_fifo, &tx_data);

				i++;
				k_mutex_unlock(&my_mutex)
				
			}
		}
    }
}