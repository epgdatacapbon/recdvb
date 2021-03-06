#ifndef _RECPT1_UTIL_H_
#define _RECPT1_UTIL_H_

#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <libgen.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "config.h"
#include "decoder.h"
#include "mkpath.h"
#include "recpt1.h"
#include "tssplitter_lite.h"

/* ipc message size */
#define MSGSZ 255

/* used in checksigna.c */
#define MAX_RETRY (2)

/* type definitions */
typedef int boolean;

typedef struct sock_data {
	int sfd; /* socket fd */
	struct sockaddr_in addr;
} sock_data;

typedef struct msgbuf {
	long mtype;
	char mtext[MSGSZ];
} message_buf;

typedef struct thread_data {
	int tfd; /* tuner fd */  //xxx variable

	int fefd;   // DVB
	int dmxfd;  // DVB

	int wfd; /* output file fd */  //invariable
	int lnb; /* LNB voltage */     //invariable
	int msqid;                     //invariable
	time_t start_time;             //invariable

	int recsec;  //xxx variable

	boolean indefinite;       //invaliable
	boolean tune_persistent;  //invaliable

	QUEUE_T *queue;           //invariable
	CHANNEL_SET *table;       //invariable
	sock_data *sock_data;     //invariable
	pthread_t signal_thread;  //invariable
	decoder *decoder;         //invariable
	decoder_options *dopt;    //invariable
	splitter *splitter;       //invariable
} thread_data;

extern const char *version;

extern ISDB_FREQ_CONV_TABLE isdb_conv_table[];
extern CHANNEL_SET channel_set;
extern boolean f_exit;

/* prototypes */
int tune(char *channel, thread_data *tdata, int dev_num);
int set_frequency(thread_data *tdata, boolean);
int close_tuner(thread_data *tdata);
int lnb_control(int dev_num, int lnb_vol);
void show_channels(void);
CHANNEL_SET *searchrecoff(char *channel);
void calc_cn(int fd, int type, boolean use_bell);
int parse_time(char *rectimestr, int *recsec);
void do_bell(int bell);
void modify_ch_str(char *channel);

#endif
