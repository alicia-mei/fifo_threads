#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#define STACKSIZE 1024

K_FIFO_DEFINE(my_fifo);
K_MUTEX_DEFINE(my_mutex);

struct data_item_t {
    void *fifo_reserved;   /* primeira palavra reservada para uso pela FIFO (do site do zephyr)*/
    uint32_t value;        /* valor armazenado */
};

/* Contador de itens da fifo, max 11 */
int qty_fifo = 0;

/*escritor 1*/
void producer_thread_1(int unused1, int unused2, int unused3)
{
    while (1) {
        if (qty_fifo < 11) { //se ainda há espaço na fifo
            if (k_mutex_lock(&my_mutex, K_MSEC(100)) == 0) { //pega o mutex
                /* Cria novo item pra enviar */
                struct data_item_t *tx_data = k_malloc(sizeof(struct data_item_t));

                tx_data->value = 4; /* o que o escritor 1 coloca na fifo */

                /* coloca item na fifo */
                k_fifo_put(&my_fifo, tx_data);
                qty_fifo++; // aumenta o valor do contador
                printk("Produtor 1 colocou %d na FIFO (qty_fifo = %d)\n", tx_data->value, qty_fifo);

                k_mutex_unlock(&my_mutex); //destrava o mutex
				k_msleep(1000);
            }
        } else {
            k_msleep(100); 
        }
    }
}

/* escritor 2 - mesma lógica do escritor 1*/
void producer_thread_2(int unused1, int unused2, int unused3)
{
    while (1) {
        if (qty_fifo < 11) {
            if (k_mutex_lock(&my_mutex, K_MSEC(100)) == 0) {
                struct data_item_t *tx_data = k_malloc(sizeof(struct data_item_t));

                tx_data->value = 8; 

                k_fifo_put(&my_fifo, tx_data);
                qty_fifo++;
                printk("Produtor 2 colocou %d na FIFO (qty_fifo = %d)\n", tx_data->value, qty_fifo);

                k_mutex_unlock(&my_mutex);
				k_msleep(1000);
            }
        } else {
            k_msleep(100);
        }
    }
}

/* leitor 1*/
void consumer_thread_1(int unused1, int unused2, int unused3)
{
    while (1) {
        struct data_item_t *rx_data = k_fifo_get(&my_fifo, K_FOREVER); //espera até ter alguma coisa para ler na fifo

        if (rx_data) {
            printk("Consumidor 1 tirou %d da FIFO\n", rx_data->value);

			/*trava o mutex para que só uma thread mude o valor de qty_fifo por vez*/
            if (k_mutex_lock(&my_mutex, K_MSEC(100)) == 0) {
                qty_fifo--;
                k_mutex_unlock(&my_mutex); // destrava o mutex
				k_msleep(1000);
            }

           k_free(rx_data);
        }
    }
}

/*leitor 2 - usa a mesma lógica do leitor 2*/
void consumer_thread_2(int unused1, int unused2, int unused3)
{
    printk("Consumidor 2 iniciado\n");
    while (1) {
        struct data_item_t *rx_data = k_fifo_get(&my_fifo, K_FOREVER);

        if (rx_data) {
            printk("Consumidor 2 tirou %d da FIFO\n", rx_data->value);

            if (k_mutex_lock(&my_mutex, K_MSEC(100)) == 0) {
                qty_fifo--;
                k_mutex_unlock(&my_mutex);
				k_msleep(1000);
            }

           k_free(rx_data);
        }
    }
}

/* Definir as threads */
/* Obs.: para obter as diferentes combinações, basta comentar o define das threads que não deseja utilizar*/
K_THREAD_DEFINE(producer_1_id, STACKSIZE, producer_thread_1, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(producer_2_id, STACKSIZE, producer_thread_2, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(consumer_1_id, STACKSIZE, consumer_thread_1, NULL, NULL, NULL, 7, 0, 0);
K_THREAD_DEFINE(consumer_2_id, STACKSIZE, consumer_thread_2, NULL, NULL, NULL, 7, 0, 0);
